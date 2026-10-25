#!/usr/bin/env node
'use strict';
/* Standalone interactive host terminal using RioOS's actual WASM command dispatcher. */
const fs = require('node:fs');
const os = require('node:os');
const path = require('node:path');
const readline = require('node:readline');

const wasmPath = path.join(__dirname, 'dist', 'rioos.wasm');
const dataDir = process.env.RIOOS_DATA_DIR || path.join(os.homedir(), '.rioos-wasm32');
const diskPath = path.join(dataDir, 'myfs-sectors.json');
const SECTOR_SIZE = 512;
const sectors = new Map();
let instance = null;
let isSaving = false;

function loadDisk() {
  try {
    const doc = JSON.parse(fs.readFileSync(diskPath, 'utf8'));
    const entries = doc && doc.sectors && typeof doc.sectors === 'object' ? Object.entries(doc.sectors) : [];
    for (const [key, encoded] of entries) {
      if (!/^\d+$/.test(key) || typeof encoded !== 'string') continue;
      const bytes = Buffer.from(encoded, 'base64');
      if (bytes.length === SECTOR_SIZE) sectors.set(Number(key) >>> 0, Uint8Array.from(bytes));
    }
  } catch (error) {
    if (error.code !== 'ENOENT') console.error(`rioos: could not load MyFS image: ${error.message}`);
  }
}

function saveDisk() {
  if (isSaving) return;
  isSaving = true;
  try {
    fs.mkdirSync(dataDir, { recursive: true });
    const stored = {};
    for (const [sector, bytes] of sectors) stored[String(sector)] = Buffer.from(bytes).toString('base64');
    const temporary = `${diskPath}.tmp`;
    fs.writeFileSync(temporary, JSON.stringify({ format: 'rioos-wasm32-myfs-v1', sectors: stored }));
    fs.renameSync(temporary, diskPath);
  } catch (error) {
    console.error(`\nrioos: warning: could not save MyFS: ${error.message}`);
  } finally {
    isSaving = false;
  }
}

function createImports(getInstance) {
  return { env: {
    rio_host_console_putc(value) {
      process.stdout.write(Buffer.from([value & 0xff]));
      return 0;
    },
    rio_host_storage_read(sector, ptr) {
      const target = new Uint8Array(getInstance().exports.memory.buffer, ptr >>> 0, SECTOR_SIZE);
      const bytes = sectors.get(sector >>> 0);
      if (bytes) target.set(bytes); else target.fill(0);
      return 0;
    },
    rio_host_storage_write(sector, ptr) {
      const source = new Uint8Array(getInstance().exports.memory.buffer, ptr >>> 0, SECTOR_SIZE);
      const key = sector >>> 0;
      if (source.some(byte => byte !== 0)) sectors.set(key, source.slice());
      else sectors.delete(key);
      return 0;
    },
    rio_host_epoch_seconds() { return Math.floor(Date.now() / 1000) >>> 0; },
    rio_host_monotonic_millis() { return Math.floor(performance.now()) >>> 0; },
    rio_host_prompt_password(ptr, capacity) {
      new Uint8Array(getInstance().exports.memory.buffer, ptr >>> 0, capacity >>> 0).fill(0);
      return -1;
    }
  }};
}

function readCString(ptr) {
  const memory = new Uint8Array(instance.exports.memory.buffer);
  const start = ptr >>> 0;
  let end = start;
  while (end < memory.length && memory[end] !== 0) end++;
  return new TextDecoder().decode(memory.subarray(start, end));
}

function getCwd() {
  return readCString(instance.exports.rio_cli_cwd_ptr());
}

function showPrompt(rl) {
  const cwd = getCwd() || '~';
  rl.setPrompt(`\x1b[32mrio@rioos\x1b[0m:\x1b[34m${cwd}\x1b[0m $ `);
  rl.prompt();
}

async function main() {
  if (!fs.existsSync(wasmPath)) {
    throw new Error(`Missing ${wasmPath}. Run bash wasm/build.sh first.`);
  }
  loadDisk();
  const bytes = fs.readFileSync(wasmPath);
  let pending;
  const module = await WebAssembly.compile(bytes);
  pending = await WebAssembly.instantiate(module, createImports(() => instance));
  instance = pending;
  const initStatus = instance.exports.rio_init();
  if (initStatus < 0) throw new Error(`RioOS initialization failed (${initStatus}).`);
  instance.exports.rio_finish_boot();
  saveDisk();

  process.stdout.write('\x1b[2J\x1b[H');
  process.stdout.write('\x1b[32m  ____  _       ____   _____ ____\n |  _ \\(_) ___ / ___| | ____/ ___|\n | |_) | |/ _ \\___ \\ |  _| \\___ \\ \n |  _ <| | (_) |___) || |___ ___) |\n |_| \\_\\_|\\___/|____/ |_____|____/\x1b[0m\n');
  process.stdout.write('RioOS WASM32 · standalone terminal edition\n');
  process.stdout.write('Built-in RioOS shell + MyFS persistent storage. Type `help` for commands.\n');
  process.stdout.write(`MyFS: ${sectors.size} saved sectors · ${dataDir}\n`);
  process.stdout.write('Note: browser-only UI apps can be launched with: bash run-ui.sh\n\n');

  const rl = readline.createInterface({ input: process.stdin, output: process.stdout, terminal: true, historySize: 1000 });
  let closing = false;
  const close = () => {
    if (closing) return;
    closing = true;
    saveDisk();
    rl.close();
  };
  rl.on('SIGINT', () => {
    process.stdout.write('^C\n');
    showPrompt(rl);
  });
  rl.on('close', () => {
    saveDisk();
    process.stdout.write('\nRioOS terminal closed. MyFS saved.\n');
  });
  showPrompt(rl);
  rl.on('line', (rawLine) => {
    const line = rawLine.replace(/\r?\n$/, '');
    const trimmed = line.trim();
    if (!trimmed) { showPrompt(rl); return; }
    if (/^(exit|logout|quit)$/i.test(trimmed)) { close(); return; }
    if (/^(nano|calc|exec|userdemo|user|su)(?:\s|$)/i.test(trimmed)) {
      process.stdout.write(`${trimmed.split(/\s+/, 1)[0]}: interactive ELF execution is not available in the WASM32 host terminal. Use bash run-ui.sh for RioOS's graphical apps.\n`);
      showPrompt(rl);
      return;
    }
    const encoded = Buffer.from(line, 'utf8');
    if (encoded.length > 1023) {
      process.stdout.write('rioos: command too long (maximum 1023 UTF-8 bytes)\n');
      showPrompt(rl);
      return;
    }
    const ptr = instance.exports.rio_cli_buffer_ptr() >>> 0;
    const target = new Uint8Array(instance.exports.memory.buffer, ptr, 1024);
    target.fill(0);
    target.set(encoded);
    const result = instance.exports.rio_cli_execute(encoded.length);
    if (result < 0) process.stdout.write(`rioos: command failed (${result})\n`);
    saveDisk();
    showPrompt(rl);
  });
}

main().catch(error => {
  console.error(`RioOS terminal startup error: ${error.stack || error}`);
  process.exitCode = 1;
});
