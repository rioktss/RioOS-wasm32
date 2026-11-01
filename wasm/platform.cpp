#include "types.h"
#include "uart.h"
#include "storage.h"
#include "rtc.h"
#include "timer.h"
#include "interrupt.h"
#include "mmu.h"
#include "process.h"
#include "scheduler.h"
#include "thread.h"
#include "security.h"
#include "smp.h"
#include "virtio_net.h"
#include "diag.h"
#include "fd.h"
#include "syscall.h"
#include "user.h"
#include "elf.h"
#include "version.h"
#include "fs.h"
#include "string.h"
#include "memory.h"
#include "console.h"

extern "C" int rio_host_console_putc(int value);
extern "C" int rio_host_storage_read(uint32_t sector, void* destination);
extern "C" int rio_host_storage_write(uint32_t sector, const void* source);
extern "C" uint32_t rio_host_epoch_seconds();
extern "C" uint32_t rio_host_monotonic_millis();
extern "C" int rio_host_prompt_password(char* destination, uint32_t capacity);

static uart_output_hook_t active_output_hook = 0;
static uart_output_flush_hook_t active_flush_hook = 0;
static int mirror_output = 0;
static int storage_is_ready = 0;
static uint64_t time_override = 0;
static uint32_t time_override_start = 0;
static int time_is_overridden = 0;
static uint32_t timer_hz = 0;
static uint32_t timer_state = TIMER_MODE_OFF;
static uint32_t timer_deadline = 0;
static uint32_t timer_start_ms = 0;
static uint64_t oneshot_events = 0;
static int scheduler_tick_enabled = 1;
static int irq_enabled = 0;
static SecurityRole current_role = SECURITY_USER;

static int str_same(const char* a, const char* b)
{
    if (!a || !b) return 0;
    int i = 0;
    while (a[i] && b[i] && a[i] == b[i]) ++i;
    return a[i] == '\0' && b[i] == '\0';
}

static void timer_refresh()
{
    if (timer_state == TIMER_MODE_ONESHOT &&
        (int32_t)(rio_host_monotonic_millis() - timer_deadline) >= 0)
    {
        timer_state = TIMER_MODE_OFF;
        timer_hz = 0;
        ++oneshot_events;
    }
}

void uart_init() {}
int console_start() { return -1; }
int console_active() { return 0; }
void console_putc(char) {}
void console_write(const char*) {}
void console_flush() {}
void console_scroll_lines(int) {}
void console_scroll_pages(int) {}
void console_snap_to_bottom() {}
int console_scroll_offset() { return 0; }
int console_columns() { return 80; }
int console_rows() { return 25; }
int console_scrollback_lines() { return 0; }
void console_write_attr(const char* text, int, int) { uart_puts(text); }
void uart_putc(char value)
{
    if (active_output_hook) active_output_hook(value);
    if (!active_output_hook || mirror_output)
        (void)rio_host_console_putc((unsigned char)value);
}
void uart_puts(const char* value)
{
    if (!value) return;
    while (*value) uart_putc(*value++);
}
void uart_putc_serial_only(char value) { (void)rio_host_console_putc((unsigned char)value); }
void uart_puts_serial_only(const char* value)
{
    if (!value) return;
    while (*value) uart_putc_serial_only(*value++);
}
int uart_try_getc() { return -1; }
int uart_has_data() { return 0; }
char uart_getc() { return 0; }
void uart_set_output_hook(uart_output_hook_t hook, int mirror_to_serial)
{ active_output_hook = hook; mirror_output = mirror_to_serial ? 1 : 0; }
void uart_clear_output_hook() { active_output_hook = 0; mirror_output = 0; }
int uart_output_hook_active() { return active_output_hook != 0; }
void uart_set_output_flush_hook(uart_output_flush_hook_t hook) { active_flush_hook = hook; }
void uart_flush_output() { if (active_flush_hook) active_flush_hook(); }

void storage_init() { storage_is_ready = 1; }
int storage_ready() { return storage_is_ready; }
int storage_read_sector(uint64_t sector, void* buffer)
{
    if (!storage_is_ready || !buffer || sector >= 32768ULL) return -1;
    return rio_host_storage_read((uint32_t)sector, buffer);
}
int storage_write_sector(uint64_t sector, const void* buffer)
{
    if (!storage_is_ready || !buffer || sector >= 32768ULL) return -1;
    return rio_host_storage_write((uint32_t)sector, buffer);
}
int storage_flush() { return 0; }
void storage_status()
{ uart_puts("Storage backend: browser localStorage sectors (MyFS 16 MiB logical disk)\r\n"); }

uint64_t rtc_get_epoch()
{
    if (time_is_overridden)
        return time_override + (uint64_t)(uint32_t)(rio_host_monotonic_millis() - time_override_start) / 1000ULL;
    return rio_host_epoch_seconds();
}
int rtc_set_epoch(uint64_t epoch)
{
    if (epoch > 4102444799ULL) return -1;
    time_override = epoch;
    time_override_start = rio_host_monotonic_millis();
    time_is_overridden = 1;
    return 0;
}

void timer_init()
{
    timer_hz = 0;
    timer_state = TIMER_MODE_OFF;
    timer_deadline = 0;
    timer_start_ms = rio_host_monotonic_millis();
    oneshot_events = 0;
}
void timer_start_periodic(uint32_t hz)
{
    timer_hz = hz;
    timer_state = hz ? TIMER_MODE_PERIODIC : TIMER_MODE_OFF;
}
void timer_start_oneshot_ms(uint32_t ms)
{
    if (!ms) { timer_stop(); return; }
    timer_hz = 0;
    timer_deadline = rio_host_monotonic_millis() + ms;
    timer_state = TIMER_MODE_ONESHOT;
}
void timer_stop() { timer_hz = 0; timer_state = TIMER_MODE_OFF; }
void timer_interrupt_handler() { /* Browser event loop has no guest IRQ delivery. */ }
void timer_set_sched_tick(int enabled) { scheduler_tick_enabled = enabled ? 1 : 0; }
uint32_t timer_mode() { timer_refresh(); return timer_state; }
uint32_t timer_current_hz() { return timer_hz; }
uint64_t timer_irq_count() { return 0; }
uint64_t timer_stray_count() { return 0; }
uint64_t timer_oneshot_fired() { timer_refresh(); return oneshot_events; }
uint64_t timer_ticks() { return timer_millis(); }
uint64_t timer_frequency() { return 1000ULL; }
uint64_t timer_millis() { return (uint64_t)(uint32_t)(rio_host_monotonic_millis() - timer_start_ms); }
uint64_t timer_seconds() { return timer_millis() / 1000ULL; }
void timer_delay_ms(uint64_t ms)
{
    const uint64_t start = timer_millis();
    while (timer_millis() - start < ms) {}
}

void interrupt_init() { irq_enabled = 0; }
void interrupt_enable() { irq_enabled = 1; }
void interrupt_disable() { irq_enabled = 0; }
uint64_t interrupt_count() { return 0; }
uint32_t interrupt_last_irq() { return 0; }
void interrupt_get_stats(InterruptStats* out)
{
    if (!out) return;
    out->total = out->timer = out->spurious = out->unhandled = out->nested = 0;
    out->last_irq = out->max_depth = 0;
}
int interrupt_cpu_irq_enabled() { return irq_enabled; }
void interrupt_idle() {}
void interrupt_timer_line_enable() {}
void interrupt_timer_line_disable() {}
extern "C" void interrupt_dispatch(ExceptionFrame*) {}
extern "C" void interrupt_exception(uint64_t, uint64_t, uint64_t, uint64_t) {}

int mmu_enabled() { return 0; }
uint64_t mmu_read_sctlr() { return 0; }
uint64_t mmu_read_ttbr0() { return 0; }
uint64_t mmu_read_tcr() { return 0; }
void mmu_init() {}
void mmu_status()
{ uart_puts("Memory protection: WebAssembly linear-memory sandbox; guest ARM MMU is not present.\r\n"); }
int mmu_user_prepare() { return -1; }
int mmu_user_set_range(uint64_t, uint64_t, uint32_t) { return -1; }
int mmu_user_finish() { return -1; }
int mmu_user_pointer_ok(uint64_t address, uint64_t length, int)
{
    const uint64_t total = memory_total();
    return address != 0 && address < total && length <= total - address;
}
uint64_t mmu_user_stack_top() { return 0; }
int mmu_user_space_create(int) { return -1; }
int mmu_user_space_map(int, uint64_t, uint64_t, uint32_t) { return -1; }
int mmu_user_space_copy_to(int, uint64_t, const void*, uint64_t) { return -1; }
void* mmu_user_space_pointer(int, uint64_t) { return 0; }
int mmu_user_space_finish(int) { return -1; }
int mmu_user_space_activate(int) { return -1; }
int mmu_user_space_deactivate() { return -1; }
int mmu_user_space_destroy(int) { return -1; }
int mmu_user_space_sync_code(int, uint64_t, uint64_t) { return -1; }
int mmu_user_space_exists(int) { return 0; }
int mmu_user_space_active_pid() { return -1; }

uint64_t fault_user_count();
uint64_t fault_kernel_count();

uint32_t smp_cpu_id() { return 0; }
uint32_t smp_online_cpu_count() { return 1; }
int smp_is_enabled() { return 0; }
void smp_init() {}
void smp_spinlock_init(SmpSpinlock* lock) { if (lock) __atomic_store_n(&lock->value, 0U, __ATOMIC_RELAXED); }
void smp_spin_lock(SmpSpinlock* lock)
{ if (lock) while (__atomic_exchange_n(&lock->value, 1U, __ATOMIC_ACQUIRE) != 0U) {} }
int smp_spin_try_lock(SmpSpinlock* lock)
{
    if (!lock) return 0;
    uint32_t expected = 0;
    return __atomic_compare_exchange_n(&lock->value, &expected, 1U, 0, __ATOMIC_ACQUIRE, __ATOMIC_RELAXED) ? 1 : 0;
}
void smp_spin_unlock(SmpSpinlock* lock)
{ if (lock) __atomic_store_n(&lock->value, 0U, __ATOMIC_RELEASE); }

void thread_init() {}
int thread_create(const char*, ThreadEntry, void*, uint32_t) { return -1; }
void thread_yield() {}
void thread_sleep_ms(uint64_t) {}
int thread_current() { return 0; }
void thread_set_preempt(int) {}
int thread_preempt_enabled() { return 0; }
void thread_timer_tick() {}
void thread_preempt_check() {}
int thread_active_count() { return 1; }
uint64_t thread_switch_count() { return 0; }
void thread_list() { uart_puts("Threads: WebAssembly build uses a single browser event loop; native stack switching is unavailable.\r\n"); }
extern "C" void thread_exit() {}
extern "C" void thread_entry_hook() {}
extern "C" void thread_context_switch(uint64_t*, uint64_t) {}
extern "C" void thread_trampoline() {}

void security_init() { current_role = SECURITY_USER; fs_set_credentials(1000U, 1000U); }
SecurityRole security_role() { return current_role; }
int security_is_root() { return current_role == SECURITY_ROOT; }
const char* security_username() { return security_is_root() ? "root" : "user"; }
char security_prompt_char() { return security_is_root() ? '#' : '$'; }
int security_login_user(const char* password)
{ if (!str_same(password, "123")) return -1; current_role = SECURITY_USER; fs_set_credentials(1000U, 1000U); return 0; }
int security_login_root(const char* password)
{ if (!str_same(password, "123")) return -1; current_role = SECURITY_ROOT; fs_set_credentials(0U, 0U); return 0; }
void security_logout_to_user() { current_role = SECURITY_USER; fs_set_credentials(1000U, 1000U); }
int security_read_password(char* out, uint64_t cap)
{ if (!out || cap < 2 || cap > 4096) return -1; return rio_host_prompt_password(out, (uint32_t)cap); }

void virtio_net_init() {}
int virtio_net_present() { return 0; }
int virtio_net_ready() { return 0; }
void virtio_net_get_mac(uint8_t mac[6]) { if (mac) for (int i = 0; i < 6; ++i) mac[i] = 0; }
int virtio_net_send(const void*, uint32_t) { return -1; }
int virtio_net_receive(void*, uint32_t) { return -1; }
uint64_t virtio_net_base() { return 0; }
int virtio_net_irq() { return -1; }
void virtio_net_status() { uart_puts("VirtIO-Net: unavailable in browser WASM32 runtime.\r\n"); }

int user_current_pid() { return 1; }
const char* user_get_args() { return ""; }
void user_set_exit_code(int) {}
void user_execute(const char*, const char*)
{ uart_puts("ELF execution: native AArch64 binaries cannot run in wasm32; use built-in RioOS applications.\r\n"); }
extern "C" void user_exit_return() {}
extern "C" void user_enter(uint64_t, uint64_t) {}
int elf_load(const char*, uint64_t*) { return -1; }
int elf_load_for_process(const char*, uint64_t*, int) { return -1; }
void elf_status() { uart_puts("ELF loader: native AArch64 ELF execution is not supported by the WASM32 browser target.\r\n"); }

uint64_t syscall_invoke(uint64_t number, uint64_t a0, uint64_t a1, uint64_t a2,
                        uint64_t a3, uint64_t a4, uint64_t a5)
{
    ExceptionFrame frame;
    for (int i = 0; i < 31; ++i) frame.x[i] = 0;
    frame.x[0] = a0; frame.x[1] = a1; frame.x[2] = a2; frame.x[3] = a3;
    frame.x[4] = a4; frame.x[5] = a5; frame.x[8] = number;
    frame.elr = frame.spsr = frame.esr = frame.far = frame.padding = 0;
    syscall_dispatch(&frame, 0);
    return frame.x[0];
}

extern "C" void command_stub_init()
{
    memory_init();
    storage_init();
    process_init();
    scheduler_init();
    fd_init();
    syscall_init();
    timer_init();
    interrupt_init();
    diag_set_level(LOG_WARN);
}

/* Capability flags are queried by the browser shell; they never imply that
   unavailable AArch64 hardware features are emulated. */
extern "C" int rio_wasm_platform_ready() { return 1; }
extern "C" int rio_wasm_network_supported() { return 0; }
extern "C" int rio_wasm_irq_supported() { return 0; }
extern "C" int rio_wasm_storage_supported() { return 1; }
extern "C" int rio_wasm_process_metadata_supported() { return 1; }
extern "C" int rio_wasm_page_allocator_supported() { return 1; }
