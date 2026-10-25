#!/usr/bin/env node
'use strict';
// Offline WASM runtime smoke test. Sector Map is a deterministic stand-in for localStorage.
const fs = require('node:fs');
const path = require('node:path');
const assert = require('node:assert/strict');

const wasmPath = path.join(__dirname, 'dist', 'rioos.wasm');
const bytes = fs.readFileSync(wasmPath);
const startedAt = Date.now();
const sectors = new Map();
let lastConsole = '';

function createImports(getInstance) {
  return { env: {
    rio_host_console_putc(value) { lastConsole += String.fromCharCode(value & 255); return 0; },
    rio_host_storage_read(sector, ptr) {
      const mem = new Uint8Array(getInstance().exports.memory.buffer, ptr >>> 0, 512);
      const value = sectors.get(sector >>> 0);
      if (value) mem.set(value); else mem.fill(0);
      return 0;
    },
    rio_host_storage_write(sector, ptr) {
      const value = new Uint8Array(getInstance().exports.memory.buffer, ptr >>> 0, 512).slice();
      if (value.some(byte => byte !== 0)) sectors.set(sector >>> 0, value);
      else sectors.delete(sector >>> 0);
      return 0;
    },
    rio_host_epoch_seconds() { return Math.floor(Date.now() / 1000) >>> 0; },
    rio_host_monotonic_millis() { return (Date.now() - startedAt) >>> 0; },
    rio_host_prompt_password(ptr, capacity) {
      new Uint8Array(getInstance().exports.memory.buffer, ptr >>> 0, capacity >>> 0).fill(0);
      return -1; // Prompt cancelled in unattended tests.
    }
  }};
}

function framebufferHash(exports) {
  const ptr = exports.rio_framebuffer_ptr() >>> 0;
  const pixels = new Uint32Array(exports.memory.buffer, ptr,
    exports.rio_framebuffer_width() * exports.rio_framebuffer_height());
  let hash = 2166136261;
  for (let i = 0; i < pixels.length; i += 17) {
    hash ^= pixels[i] >>> 0;
    hash = Math.imul(hash, 16777619) >>> 0;
  }
  assert.ok(pixels.some(value => value !== 0), 'framebuffer should not be blank');
  return hash >>> 0;
}

async function main() {
  const module = await WebAssembly.compile(bytes);
  assert.deepEqual(WebAssembly.Module.imports(module).map(item => item.name).sort(), [
    'rio_host_console_putc', 'rio_host_epoch_seconds', 'rio_host_monotonic_millis',
    'rio_host_prompt_password', 'rio_host_storage_read', 'rio_host_storage_write'
  ].sort(), 'the module should only import the documented host bridge');
  const exportsRequired = [
    'memory', 'rio_init', 'rio_finish_boot', 'rio_key', 'rio_pointer', 'rio_tick',
    'rio_framebuffer_ptr', 'rio_framebuffer_width', 'rio_framebuffer_height',
    'rio_take_dirty', 'rio_get_ui_state', 'rio_get_active_app', 'rio_seed_assets', 'rio_terminal_selftest_flags',
    'rio_cli_buffer_ptr', 'rio_cli_cwd_ptr', 'rio_cli_execute'
  ];
  for (const name of exportsRequired) assert.ok(WebAssembly.Module.exports(module).some(item => item.name === name), `missing export ${name}`);

  let first;
  first = await WebAssembly.instantiate(module, createImports(() => first));
  const e = first.exports;
  assert.equal(e.rio_init(), 0, 'first boot and asset seed should succeed');
  assert.equal(e.rio_framebuffer_width(), 1024);
  assert.equal(e.rio_framebuffer_height(), 768);
  e.rio_finish_boot();
  assert.equal(e.rio_get_ui_state(), 0, 'boot should reach lock screen');
  const lockHash = framebufferHash(e);

  e.rio_key(13); // unlock -> home
  assert.equal(e.rio_get_ui_state(), 1);
  const homeHash = framebufferHash(e);
  assert.notEqual(homeHash, lockHash, 'home screen should render differently from lock screen');

  // Exercise all seven installed built-in apps and their back navigation.
  const appHashes = new Set();
  for (let app = 1; app <= 7; app++) {
    e.rio_key(48 + app);
    assert.equal(e.rio_get_ui_state(), 2, `app ${app} should open`);
    assert.equal(e.rio_get_active_app(), app - 1, `app ${app} should be active`);
    appHashes.add(framebufferHash(e));
    e.rio_key(27); // Escape -> home
    assert.equal(e.rio_get_ui_state(), 1, `app ${app} should return to Home`);
  }
  assert.equal(appHashes.size, 7, 'each built-in app should render a distinct frame');

  // Exercise the real command dispatcher through the pixel Terminal.
  e.rio_key(49); // Terminal
  for (const ch of 'selftest') e.rio_key(ch.charCodeAt(0));
  e.rio_key(13);
  assert.equal(e.rio_get_active_app(), 0);
  framebufferHash(e);
  const selftestFlags = e.rio_terminal_selftest_flags();
  assert.ok(selftestFlags & 1, 'selftest summary should appear in Terminal scrollback');
  assert.ok(selftestFlags & 2, 'all six WASM checks should pass');
  assert.equal(selftestFlags & 4, 0, 'WASM selftest should not report a failed check');

  // Exercise the standalone host-terminal API through the same real command dispatcher.
  function runCli(command) {
    const encoded = new TextEncoder().encode(command);
    assert.ok(encoded.length <= 1023, 'test command fits CLI buffer');
    const ptr = e.rio_cli_buffer_ptr() >>> 0;
    const input = new Uint8Array(e.memory.buffer, ptr, 1024);
    input.fill(0);
    input.set(encoded);
    lastConsole = '';
    assert.equal(e.rio_cli_execute(encoded.length), 0, `CLI command should execute: ${command}`);
    return lastConsole;
  }
  assert.match(runCli('pwd'), /\/home\/user/, 'standalone CLI reports the VFS current directory');
  assert.match(runCli('echo RIOOS_CLI_TEST_OK'), /RIOOS_CLI_TEST_OK/, 'standalone CLI writes actual dispatcher output');
  runCli('echo RIOOS_PERSISTENCE_OK > /home/user/wasm-cli-test.txt');
  assert.match(runCli('cat /home/user/wasm-cli-test.txt'), /RIOOS_PERSISTENCE_OK/, 'standalone CLI file redirection and cat work');
  assert.match(runCli('cd /'), /\r?\n/, 'standalone CLI command can change directory');
  const cwdPtr = e.rio_cli_cwd_ptr() >>> 0;
  const cwdMem = new Uint8Array(e.memory.buffer);
  let cwdEnd = cwdPtr;
  while (cwdEnd < cwdMem.length && cwdMem[cwdEnd] !== 0) cwdEnd++;
  assert.equal(new TextDecoder().decode(cwdMem.subarray(cwdPtr, cwdEnd)), '/', 'CLI prompt path follows cd');

  const savedSectorCount = sectors.size;
  assert.ok(savedSectorCount > 0, 'MyFS should have written persistent sectors');

  // Instantiate a fresh WASM instance over the same sectors to verify reload compatibility.
  let second;
  second = await WebAssembly.instantiate(module, createImports(() => second));
  assert.equal(second.exports.rio_init(), 0, 'filesystem should mount after reload');
  second.exports.rio_finish_boot();
  second.exports.rio_key(13);
  for (let app = 1; app <= 7; app++) {
    second.exports.rio_key(48 + app);
    assert.equal(second.exports.rio_get_ui_state(), 2, `reloaded app ${app} should open`);
    assert.equal(second.exports.rio_get_active_app(), app - 1);
    second.exports.rio_key(27);
  }

  console.log('PASS: WASM imports/exports and module validation');
  console.log('PASS: boot, lock screen, Home, framebuffer rendering');
  console.log('PASS: all 7 built-in apps and Escape navigation');
  console.log('PASS: Terminal command dispatcher runs `selftest` with no failed WASM checks');
  console.log('PASS: host terminal API (pwd, echo, redirect, cat, cd) uses the real command dispatcher');
  console.log(`PASS: persistence reload (${savedSectorCount} non-zero sectors; all 7 apps reopen)`);
  console.log(`PASS: runtime smoke checks (${Date.now() - startedAt} ms); host output bytes=${lastConsole.length}`);
}

main().catch(error => { console.error(error.stack || error); process.exitCode = 1; });
