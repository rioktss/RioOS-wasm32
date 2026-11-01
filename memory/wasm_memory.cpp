#include "memory.h"
#include "page_alloc.h"
#include "types.h"

/* WebAssembly owns the actual linear-memory mapping. RioOS provides bounded
   kernel heap and page-pool arenas inside that mapping for portable modules. */
static uint8_t heap_arena[2U * 1024U * 1024U] __attribute__((aligned(16)));
static uint8_t page_arena[2U * 1024U * 1024U] __attribute__((aligned(4096)));
static uint64_t heap_cursor = 0;
static int memory_ready = 0;

static uint64_t align16(uint64_t value)
{
    return (value + 15ULL) & ~15ULL;
}

void memory_init()
{
    heap_cursor = 0;
    memory_ready = page_alloc_init(page_arena, sizeof(page_arena)) == 0;
}

void* kmalloc(uint64_t size)
{
    if (!memory_ready || size == 0) return 0;
    const uint64_t start = align16(heap_cursor);
    if (size > sizeof(heap_arena) || start > sizeof(heap_arena) - size) return 0;
    const uint64_t end = start + size;
    heap_cursor = align16(end);
    if (heap_cursor > sizeof(heap_arena)) heap_cursor = sizeof(heap_arena);
    uint8_t* result = heap_arena + start;
    for (uint64_t i = 0; i < size; ++i) result[i] = 0;
    return result;
}

uint64_t memory_total()
{
#if defined(__wasm32__)
    return (uint64_t)__builtin_wasm_memory_size(0) * 65536ULL;
#else
    return 0;
#endif
}

uint64_t heap_total() { return sizeof(heap_arena); }
uint64_t heap_used() { return heap_cursor; }
uint64_t heap_free() { return heap_cursor >= sizeof(heap_arena) ? 0 : sizeof(heap_arena) - heap_cursor; }
