#include "types.h"
#include "fs.h"
#include "vfs.h"
#include "history.h"
#include "env.h"
#include "security.h"
#include "fd.h"
#include "commands.h"
#include "uart.h"

static int initialized = 0;
static char cli_command_buffer[1024];
static char cli_cwd_buffer[192];

extern "C" void command_stub_init();

extern "C" int rio_init()
{
    if (initialized) return 0;

    command_stub_init();
    fs_init();
    security_init();
    vfs_init();
    history_init();

    (void)env_set("HOME=/home/user");
    (void)env_set("USER=user");
    (void)env_set("SHELL=rioos");
    (void)env_set("PATH=/bin");
    (void)fd_set_process(0);

    initialized = 1;
    return 0;
}

extern "C" void rio_finish_boot()
{
    /* The text shell is ready immediately after rio_init(). */
}

extern "C" uint32_t rio_cli_buffer_ptr()
{
    return (uint32_t)cli_command_buffer;
}

extern "C" uint32_t rio_cli_cwd_ptr()
{
    vfs_get_path(cli_cwd_buffer, (int)sizeof(cli_cwd_buffer));
    return (uint32_t)cli_cwd_buffer;
}

extern "C" int rio_cli_execute(int length)
{
    if (!initialized) return -1;
    if (length < 0 || length >= (int)sizeof(cli_command_buffer))
    {
        uart_puts("rioos: command too long (max 1023 bytes)\r\n");
        return -2;
    }
    cli_command_buffer[length] = '\0';
    if (length == 0) return 0;
    execute_command(cli_command_buffer);
    uart_puts("\r\n");
    return 0;
}
