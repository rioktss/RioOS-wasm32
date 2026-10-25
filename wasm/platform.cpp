#include "types.h"
#include "uart.h"
#include "storage.h"
#include "rtc.h"
#include "memory.h"
#include "timer.h"
#include "interrupt.h"
#include "mmu.h"
#include "page_alloc.h"
#include "process.h"
#include "scheduler.h"
#include "thread.h"
#include "security.h"
#include "smp.h"
#include "virtio_net.h"
#include "netstack.h"
#include "diag.h"
#include "fault.h"
#include "fd.h"
#include "syscall.h"
#include "user.h"
#include "elf.h"
#include "version.h"
#include "fs.h"
#include "string.h"

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
static uint32_t time_override = 0;
static int time_is_overridden = 0;
static int current_log_level = LOG_INFO;
static SecurityRole current_role = SECURITY_USER;
static uint32_t irq_timer_hz = 0;
static uint32_t irq_timer_mode = TIMER_MODE_OFF;
static uint64_t boot_millis = 0;

struct WasmProcessRecord { int used; int pid; int state; char name[32]; };
static WasmProcessRecord process_records[16];
static int next_process_pid = 1;
static int scheduler_ready[16];
static uint64_t logical_ticks = 0;

static void put_u64(uint64_t v)
{
    char buf[24]; int n = 0;
    if (v == 0) { uart_putc('0'); return; }
    while (v && n < (int)sizeof(buf)) { buf[n++] = (char)('0' + v % 10ULL); v /= 10ULL; }
    while (n) uart_putc(buf[--n]);
}
static int str_same(const char* a, const char* b)
{
    if (!a || !b) return 0;
    int i = 0; while (a[i] && b[i] && a[i] == b[i]) ++i;
    return a[i] == '\0' && b[i] == '\0';
}

void uart_putc(char c)
{
    if (active_output_hook) active_output_hook(c);
    if (!active_output_hook || mirror_output) (void)rio_host_console_putc((unsigned char)c);
}
void uart_puts(const char* s) { if (!s) return; while (*s) uart_putc(*s++); }
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
{
    uart_puts("Storage backend: browser localStorage sectors (MyFS 16 MiB logical disk)\r\n");
}

uint64_t rtc_get_epoch()
{
    uint32_t now = rio_host_epoch_seconds();
    if (time_is_overridden) return time_override + (uint64_t)(rio_host_monotonic_millis() - boot_millis) / 1000ULL;
    return now;
}
int rtc_set_epoch(uint64_t epoch)
{
    if (epoch > 4102444799ULL) return -1;
    time_override = (uint32_t)epoch;
    boot_millis = rio_host_monotonic_millis();
    time_is_overridden = 1;
    return 0;
}

uint64_t timer_millis() { return (uint64_t)rio_host_monotonic_millis(); }
uint64_t timer_seconds() { return timer_millis() / 1000ULL; }
uint64_t timer_ticks() { return logical_ticks; }
uint64_t timer_frequency() { return 1000ULL; }
void timer_init() { irq_timer_hz = 0; irq_timer_mode = TIMER_MODE_OFF; }
void timer_start_periodic(uint32_t hz) { irq_timer_hz = hz; irq_timer_mode = hz ? TIMER_MODE_PERIODIC : TIMER_MODE_OFF; }
void timer_start_oneshot_ms(uint32_t ms) { (void)ms; irq_timer_hz = 0; irq_timer_mode = TIMER_MODE_OFF; }
void timer_stop() { irq_timer_hz = 0; irq_timer_mode = TIMER_MODE_OFF; }
void timer_interrupt_handler() {}
void timer_set_sched_tick(int enabled) { (void)enabled; }
uint32_t timer_mode() { return irq_timer_mode; }
uint32_t timer_current_hz() { return irq_timer_hz; }
uint64_t timer_irq_count() { return 0; }
uint64_t timer_stray_count() { return 0; }
uint64_t timer_oneshot_fired() { return 0; }
void timer_delay_ms(uint64_t ms) { (void)ms; }

void interrupt_init() {}
void interrupt_enable() {}
void interrupt_disable() {}
uint64_t interrupt_count() { return 0; }
uint32_t interrupt_last_irq() { return 0; }
void interrupt_get_stats(InterruptStats* out)
{
    if (!out) return;
    out->total = out->timer = out->spurious = out->unhandled = out->nested = 0;
    out->last_irq = out->max_depth = 0;
}
int interrupt_cpu_irq_enabled() { return 0; }
void interrupt_timer_line_enable() {}
void interrupt_timer_line_disable() {}

int mmu_enabled() { return 0; }
uint64_t mmu_read_sctlr() { return 0; }
uint64_t mmu_read_ttbr0() { return 0; }
uint64_t mmu_read_tcr() { return 0; }
void mmu_status() { uart_puts("MMU: not applicable; linear memory is managed by the WebAssembly runtime.\r\n"); }
int mmu_user_prepare() { return -1; }
int mmu_user_set_range(uint64_t, uint64_t, uint32_t) { return -1; }
int mmu_user_finish() { return -1; }
int mmu_user_pointer_ok(uint64_t address, uint64_t length, int)
{ return address != 0 && length <= (16ULL * 1024ULL * 1024ULL); }
uint64_t mmu_user_stack_top() { return 0; }

void page_alloc_get_stats(PageStats* out)
{
    if (!out) return;
    out->total_pages = out->free_pages = out->allocs = out->frees = 0;
    out->failed_allocs = out->rejected_frees = 0;
}
int page_alloc_ready() { return 0; }
int page_alloc_init(void*, uint64_t) { return -1; }
void* allocate_page() { return 0; }
int free_page(void*) { return PAGE_FREE_BAD_ADDRESS; }
uint64_t fault_user_count() { return 0; }
uint64_t fault_kernel_count() { return 0; }

void process_init()
{
    for (int i = 0; i < 16; ++i) { process_records[i].used = 0; process_records[i].pid = -1; process_records[i].name[0] = '\0'; scheduler_ready[i] = 0; }
    next_process_pid = 1;
    process_records[0].used = 1; process_records[0].pid = 0; process_records[0].state = PROCESS_RUNNING;
    str_copy(process_records[0].name, "rioos-wasm32", sizeof(process_records[0].name));
    scheduler_ready[0] = 1;
}
int process_create(const char* name)
{
    for (int i = 1; i < 16; ++i) if (!process_records[i].used)
    {
        process_records[i].used = 1; process_records[i].pid = next_process_pid++;
        process_records[i].state = PROCESS_READY; str_copy(process_records[i].name, name ? name : "process", sizeof(process_records[i].name));
        return process_records[i].pid;
    }
    return -1;
}
int process_destroy(int pid)
{
    if (pid == 0) return -2;
    for (int i = 1; i < 16; ++i) if (process_records[i].used && process_records[i].pid == pid)
    { process_records[i].used = 0; scheduler_ready[i] = 0; return 0; }
    return -1;
}
int process_count() { int n = 0; for (int i = 0; i < 16; ++i) if (process_records[i].used) ++n; return n; }
void process_list()
{
    uart_puts("PID  STATE      NAME\r\n---  ---------  ----------------\r\n");
    for (int i = 0; i < 16; ++i) if (process_records[i].used)
    {
        put_u64((uint64_t)process_records[i].pid); uart_puts("    ");
        uart_puts(process_records[i].state == PROCESS_RUNNING ? "RUNNING    " : process_records[i].state == PROCESS_READY ? "READY      " : "SLEEPING   ");
        uart_puts(process_records[i].name); uart_puts("\r\n");
    }
}
int process_set_state(int pid, int state)
{
    for (int i = 0; i < 16; ++i) if (process_records[i].used && process_records[i].pid == pid)
    { process_records[i].state = state; scheduler_ready[i] = state == PROCESS_READY || state == PROCESS_RUNNING; return 0; }
    return -1;
}
int process_get_state(int pid) { for (int i = 0; i < 16; ++i) if (process_records[i].used && process_records[i].pid == pid) return process_records[i].state; return PROCESS_UNUSED; }
int process_configure_user(int, uint64_t, uint64_t) { return -1; }
int process_mark_exit(int pid, int code) { (void)code; return process_destroy(pid); }
int process_is_user(int) { return 0; }
uint64_t process_user_low(int) { return 0; }
uint64_t process_user_high(int) { return 0; }
int process_exit_code(int) { return 0; }
int process_pid_at(int slot) { return slot >= 0 && slot < 16 && process_records[slot].used ? process_records[slot].pid : -1; }

void scheduler_init() { logical_ticks = 0; }
int scheduler_add(int pid) { for (int i = 0; i < 16; ++i) if (process_records[i].used && process_records[i].pid == pid) { scheduler_ready[i] = 1; return 0; } return -1; }
int scheduler_remove(int pid) { for (int i = 0; i < 16; ++i) if (process_records[i].used && process_records[i].pid == pid) { scheduler_ready[i] = 0; return 0; } return -1; }
int scheduler_set_current_logical(int pid) { return pid == 0 ? 0 : -1; }
int scheduler_current_pid() { return 0; }
int scheduler_next_pid() { return 0; }
int scheduler_tick() { ++logical_ticks; return 0; }
int scheduler_preempt_pending() { return 0; }
void scheduler_clear_preempt_pending() {}
uint64_t scheduler_ticks() { return logical_ticks; }
int scheduler_ready_count() { int n=0; for (int i=0;i<16;++i) if (process_records[i].used && scheduler_ready[i]) ++n; return n; }
void scheduler_status() { uart_puts("Scheduler: single-threaded cooperative browser event loop; no CPU context switching.\r\n"); }

uint32_t smp_cpu_id() { return 0; }
uint32_t smp_online_cpu_count() { return 1; }
int smp_is_enabled() { return 0; }
void smp_init() {}
void smp_spinlock_init(SmpSpinlock* lock) { if (lock) lock->value = 0; }
void smp_spin_lock(SmpSpinlock* lock) { if (lock) lock->value = 1; }
int smp_spin_try_lock(SmpSpinlock* lock) { if (!lock) return 0; if (lock->value) return 0; lock->value = 1; return 1; }
void smp_spin_unlock(SmpSpinlock* lock) { if (lock) lock->value = 0; }

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
void thread_list() { uart_puts("Threads: browser WebAssembly is single-threaded in this build; kernel stack switching is unavailable.\r\n"); }

void security_init() { current_role = SECURITY_USER; fs_set_credentials(1000U, 1000U); }
SecurityRole security_role() { return current_role; }
int security_is_root() { return current_role == SECURITY_ROOT; }
const char* security_username() { return current_role == SECURITY_ROOT ? "root" : "user"; }
char security_prompt_char() { return security_is_root() ? '#' : '$'; }
int security_login_user(const char* password)
{ if (!str_same(password, "123")) return -1; current_role = SECURITY_USER; fs_set_credentials(1000U,1000U); return 0; }
int security_login_root(const char* password)
{ if (!str_same(password, "rioos-root")) return -1; current_role = SECURITY_ROOT; fs_set_credentials(0U,0U); return 0; }
void security_logout_to_user() { current_role = SECURITY_USER; fs_set_credentials(1000U,1000U); }
int security_read_password(char* out, uint64_t cap)
{ if (!out || cap < 2 || cap > 4096) return -1; return rio_host_prompt_password(out, (uint32_t)cap); }

int virtio_net_present() { return 0; }
int virtio_net_ready() { return 0; }
void virtio_net_init() {}
void virtio_net_get_mac(uint8_t mac[6]) { if (mac) for (int i=0;i<6;++i) mac[i]=0; }
int virtio_net_send(const void*, uint32_t) { return -1; }
int virtio_net_receive(void*, uint32_t) { return -1; }
uint64_t virtio_net_base() { return 0; }
int virtio_net_irq() { return -1; }
void virtio_net_status() { uart_puts("VirtIO-Net: unavailable in browser WASM32 runtime.\r\n"); }

uint64_t memory_total() { return 32ULL * 1024ULL * 1024ULL; }
uint64_t heap_total() { return 4ULL * 1024ULL * 1024ULL; }
uint64_t heap_used() { return 0; }
uint64_t heap_free() { return heap_total(); }
void memory_init() {}
void* kmalloc(uint64_t) { return 0; }

int user_current_pid() { return -1; }
const char* user_get_args() { return ""; }
void user_set_exit_code(int) {}
void user_execute(const char*, const char*) { uart_puts("ELF execution: AArch64 user binaries cannot execute in wasm32; use the built-in RioOS apps.\r\n"); }
extern "C" void user_exit_return() {}
int elf_load(const char*, uint64_t*) { return -1; }
void elf_status() { uart_puts("ELF loader: AArch64 ELF execution is not supported by the wasm32 browser target.\r\n"); }

uint64_t syscall_invoke(uint64_t number, uint64_t a0, uint64_t a1, uint64_t a2,
                        uint64_t a3, uint64_t a4, uint64_t a5)
{
    ExceptionFrame frame;
    for (int i=0;i<31;++i) frame.x[i]=0;
    frame.x[0]=a0; frame.x[1]=a1; frame.x[2]=a2; frame.x[3]=a3;
    frame.x[4]=a4; frame.x[5]=a5; frame.x[8]=number;
    frame.elr=frame.spsr=frame.esr=frame.far=frame.padding=0;
    syscall_dispatch(&frame, 0);
    return frame.x[0];
}

int diag_get_level() { return current_log_level; }
void diag_set_level(int level) { if (level >= LOG_ERROR && level <= LOG_DEBUG) current_log_level = level; }
const char* diag_level_name(int level)
{ switch(level) { case LOG_ERROR:return "ERROR"; case LOG_WARN:return "WARN"; case LOG_INFO:return "INFO"; case LOG_DEBUG:return "DEBUG"; default:return "UNKNOWN"; } }
void klog(int level, const char* message)
{ if (level <= current_log_level) { uart_putc('['); uart_puts(diag_level_name(level)); uart_puts("] "); uart_puts(message); uart_puts("\r\n"); } }
uint64_t diag_log_printed() { return 0; }
uint64_t diag_log_suppressed() { return 0; }
void kpanic(const char* message, const char*, int) { uart_puts("PANIC: "); uart_puts(message ? message : "unknown"); uart_puts("\r\n"); __builtin_trap(); }

void command_version();
extern "C" void command_stub_init()
{
    storage_init();
    process_init();
    scheduler_init();
    fd_init();
    timer_init();
    interrupt_init();
    diag_set_level(LOG_INFO);
}

/* Helpers used by the WASM UI to report a truthful platform capability set. */
extern "C" int rio_wasm_platform_ready() { return 1; }
extern "C" int rio_wasm_network_supported() { return 0; }
extern "C" int rio_wasm_irq_supported() { return 0; }
