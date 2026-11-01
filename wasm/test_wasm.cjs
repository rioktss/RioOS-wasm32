#!/usr/bin/env node
'use strict';
// Terminal-only WASM runtime smoke test. Storage Map simulates the host disk.
const fs = require('node:fs');
const path = require('node:path');
const assert = require('node:assert/strict');

const wasmPath = path.join(__dirname, '..', 'rioos.wasm');
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
      return -1;
    }
  }};
}

async function main() {
  const module = await WebAssembly.compile(bytes);
  const imports = WebAssembly.Module.imports(module).map(item => item.name).sort();
  assert.deepEqual(imports, [
    'rio_host_console_putc', 'rio_host_epoch_seconds', 'rio_host_monotonic_millis',
    'rio_host_prompt_password', 'rio_host_storage_read', 'rio_host_storage_write'
  ].sort(), 'only the documented host bridge is imported');

  const exported = WebAssembly.Module.exports(module).map(item => item.name);
  for (const name of ['memory', 'rio_init', 'rio_finish_boot', 'rio_cli_buffer_ptr', 'rio_cli_cwd_ptr', 'rio_cli_execute'])
    assert.ok(exported.includes(name), `missing export ${name}`);
  for (const removed of ['rio_framebuffer_ptr', 'rio_key', 'rio_pointer', 'rio_get_ui_state'])
    assert.ok(!exported.includes(removed), `${removed} must not be present in terminal-only build`);

  let instance;
  instance = await WebAssembly.instantiate(module, createImports(() => instance));
  const e = instance.exports;
  assert.equal(e.rio_init(), 0, 'boot and filesystem initialization succeed');
  e.rio_finish_boot();

  function runCli(command) {
    const encoded = new TextEncoder().encode(command);
    assert.ok(encoded.length <= 1023, 'test command fits CLI buffer');
    const ptr = e.rio_cli_buffer_ptr() >>> 0;
    const input = new Uint8Array(e.memory.buffer, ptr, 1024);
    input.fill(0);
    input.set(encoded);
    lastConsole = '';
    assert.equal(e.rio_cli_execute(encoded.length), 0, `CLI command executes: ${command}`);
    return lastConsole;
  }

  assert.match(runCli('pwd'), /\/home\/user/, 'pwd reports initial VFS directory');
  assert.match(runCli('uname'), /wasm32 WebAssembly/, 'uname identifies WASM32');
  assert.match(runCli('version'), /MyKernel v1\.13\.0/, 'version reports synchronized kernel baseline');
  assert.match(runCli('pages'), /WASM linear-memory pages/, 'page allocator diagnostics are available');
  assert.match(runCli('mount'), /browser\/host sector bridge/, 'storage backend is identified accurately');
  assert.match(runCli('irq status'), /Hardware IRQ: unavailable/, 'no hardware IRQ is claimed');
  assert.match(runCli('smp'), /Guest CPUs.*not exposed/, 'SMP limitation is reported');
  assert.match(runCli('thread-demo'), /native kernel-thread context switching is unavailable/, 'native context switch limitation is reported');
  assert.match(runCli('apps'), /No bundled graphical applications/, 'no GUI apps are bundled');
  assert.match(runCli('fbinfo'), /text-only WASM32 host terminal/, 'runtime reports terminal-only mode');
  assert.match(runCli('echo RIOOS_CLI_TEST_OK'), /RIOOS_CLI_TEST_OK/, 'real command dispatcher works');
  runCli('echo RIOOS_PERSISTENCE_OK > /home/user/wasm-cli-test.txt');
  assert.match(runCli('cat /home/user/wasm-cli-test.txt'), /RIOOS_PERSISTENCE_OK/, 'redirection and file readback work');
  runCli('cd /');
  const cwdPtr = e.rio_cli_cwd_ptr() >>> 0;
  const cwdMem = new Uint8Array(e.memory.buffer);
  let cwdEnd = cwdPtr;
  while (cwdEnd < cwdMem.length && cwdMem[cwdEnd] !== 0) cwdEnd++;
  assert.equal(new TextDecoder().decode(cwdMem.subarray(cwdPtr, cwdEnd)), '/', 'prompt path follows cd');

  const sectorsAfterWrite = sectors.size;
  assert.ok(sectorsAfterWrite > 0, 'MyFS writes persistent sectors');
  let second;
  second = await WebAssembly.instantiate(module, createImports(() => second));
  assert.equal(second.exports.rio_init(), 0, 'filesystem initializes again with the same sector store');
  second.exports.rio_finish_boot();
  const command = 'cat /home/user/wasm-cli-test.txt';
  const encoded = new TextEncoder().encode(command);
  new Uint8Array(second.exports.memory.buffer, second.exports.rio_cli_buffer_ptr() >>> 0, 1024).set(encoded);
  lastConsole = '';
  assert.equal(second.exports.rio_cli_execute(encoded.length), 0);
  assert.match(lastConsole, /RIOOS_PERSISTENCE_OK/, 'MyFS survives runtime reload');

  console.log('PASS: WASM module imports/exports and terminal-only ABI');
  console.log('PASS: shell command dispatcher, pwd/uname/version/pages/mount');
  console.log('PASS: WASM platform limitations and no-UI/no-app mode');
  console.log('PASS: echo, redirection, cat, cd and current-directory API');
  console.log(`PASS: MyFS persistence reload (${sectorsAfterWrite} non-zero sectors)`);
  console.log(`PASS: runtime smoke checks (${Date.now() - startedAt} ms)`);
}

main().catch(error => { console.error(error.stack || error); process.exitCode = 1; });
