/*
 * Stub libcorkscrew. L removed libcorkscrew, but gralloc.mt8389.so (KK) has
 * it in DT_NEEDED and calls unwind_backtrace / get_backtrace_symbols /
 * format_backtrace_line for its debug dumps. Unwinding returns no frames,
 * so those dumps are empty; nothing else changes.
 *
 * Signatures and struct layouts from KK system/core/include/corkscrew/backtrace.h.
 */
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>

typedef struct {
    uintptr_t absolute_pc;
    uintptr_t stack_top;
    size_t stack_size;
} backtrace_frame_t;

typedef struct {
    uintptr_t relative_pc;
    uintptr_t relative_symbol_addr;
    char *map_name;
    char *symbol_name;
    char *demangled_name;
} backtrace_symbol_t;

ssize_t unwind_backtrace(backtrace_frame_t *backtrace, size_t ignore_depth, size_t max_depth)
{
    (void)backtrace; (void)ignore_depth; (void)max_depth;
    return 0;
}

ssize_t unwind_backtrace_thread(pid_t tid, backtrace_frame_t *backtrace,
                                size_t ignore_depth, size_t max_depth)
{
    (void)tid; (void)backtrace; (void)ignore_depth; (void)max_depth;
    return 0;
}

void get_backtrace_symbols(const backtrace_frame_t *backtrace, size_t frames,
                           backtrace_symbol_t *backtrace_symbols)
{
    (void)backtrace;
    if (backtrace_symbols)
        memset(backtrace_symbols, 0, frames * sizeof(*backtrace_symbols));
}

void free_backtrace_symbols(backtrace_symbol_t *backtrace_symbols, size_t frames)
{
    (void)backtrace_symbols; (void)frames;
}

void format_backtrace_line(unsigned frameNumber, const backtrace_frame_t *frame,
                           const backtrace_symbol_t *symbol, char *buffer, size_t bufferSize)
{
    (void)frameNumber; (void)frame; (void)symbol;
    if (buffer && bufferSize)
        buffer[0] = '\0';
}
