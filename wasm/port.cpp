#include "types.h"
#include "fb.h"
#include "ui.h"
#include "fs.h"
#include "vfs.h"
#include "storage.h"
#include "rtc.h"
#include "history.h"
#include "env.h"
#include "security.h"
#include "fd.h"
#include "commands.h"
#include "uart.h"

static const int RIO_W = 1024;
static const int RIO_H = 768;
static uint32_t framebuffer[RIO_W * RIO_H] __attribute__((aligned(16)));
static int initialized = 0;
static int booting = 0;
static int dirty = 0;
static int ui_state = 0; /* locked=0, home=1, app=2, control centre=3 */
static int selected_app = 0;
static int active_app = -1;
static int pointer_down = 0;
static int pointer_start_x = 0;
static int pointer_start_y = 0;
static uint64_t last_render_minute = 0;
static char cli_command_buffer[1024];
static char cli_cwd_buffer[192];

extern "C" void command_stub_init();
extern "C" int rio_seed_assets();
int fb_test_attach(uint32_t* pixels, int width, int height);

static void ui_open_app(int app_id)
{
    if (app_id < 0 || app_id >= UI_APP_COUNT || !ui_app_package_valid(app_id)) return;
    selected_app = app_id;
    active_app = app_id;
    ui_apps_init(app_id);
    ui_state = 2;
}

static void control_tile_bounds(int tile, int* x, int* y, int* width, int* height)
{
    const int margin = 64, gap = 16, tile_h = 84;
    int tile_w = (RIO_W - margin * 2 - gap) / 2;
    if (x) *x = margin + (tile % 2) * (tile_w + gap);
    if (y) *y = 218 + (tile / 2) * 102;
    if (width) *width = tile_w;
    if (height) *height = tile_h;
}

static void draw_control_centre(uint64_t epoch)
{
    const int dark = ui_settings_is_dark();
    fb_fill_gradient(dark ? 0x101827U : 0xE9EFF7U, dark ? 0x1D3450U : 0xD5E3F1U);
    fb_draw_round_rect(36, 32, RIO_W - 72, RIO_H - 64, 28, dark ? 0x17263AU : 0xF4F7FCU);
    fb_draw_ui_text("Control Centre", 66, 62, dark ? 0xFFFFFFU : 0x1E344CU, 3);
    char time_text[6]; ui_format_clock(epoch, time_text);
    fb_draw_ui_text(time_text, 66, 108, dark ? 0xD8E8FCU : 0x324C68U, 4);
    fb_draw_ui_text("WASM32 BROWSER BUILD", RIO_W - 310, 130, dark ? 0x83DDBBU : 0x3B806EU, 1);
    for (int tile = 0; tile < 6; ++tile)
    {
        int x=0,y=0,w=0,h=0; control_tile_bounds(tile,&x,&y,&w,&h);
        uint32_t color = dark ? 0x2B405BU : 0xDFE8F4U;
        if (tile == 1) color = dark ? 0x367F78U : 0xB8E1D8U;
        if (tile == 2) color = dark ? 0x354B73U : 0xC7D8F3U;
        if (tile == 3) color = dark ? 0x3D4964U : 0xD2DAEAU;
        fb_draw_round_rect(x,y,w,h,18,color);
        if (tile == 0)
        {
            fb_draw_circle(x+31,y+31,11,0xD98A80U);
            fb_draw_ui_text("Network",x+55,y+23,dark?0xFFFFFFU:0x253A54U,2);
            fb_draw_ui_text("Raw VirtIO networking unavailable",x+55,y+52,dark?0xBDD0E7U:0x5D728AU,1);
        }
        else if (tile == 1)
        {
            fb_draw_circle(x+32,y+31,12,0xE8F2FFU);
            fb_draw_ui_text("Dark mode",x+57,y+22,dark?0xFFFFFFU:0x253A54U,2);
            fb_draw_ui_text(dark?"ON - tap to switch to Light":"OFF - tap to switch to Dark",x+57,y+52,dark?0xD3E9E6U:0x4E716DU,1);
        }
        else if (tile == 2)
        {
            fb_draw_round_rect(x+18,y+19,28,31,8,0x9CBCE9U);
            fb_draw_circle(x+32,y+33,8,0xE4F3FFU);
            fb_draw_ui_text("Wallpaper",x+57,y+22,dark?0xFFFFFFU:0x253A54U,2);
            fb_draw_ui_text("Choose a local image",x+57,y+52,dark?0xBDD0E7U:0x5D728AU,1);
        }
        else if (tile == 3)
        {
            fb_draw_circle(x+32,y+31,12,0x85A5D2U);
            fb_draw_circle(x+32,y+31,5,dark?0x253A54U:0xE7EEF7U);
            fb_draw_ui_text("Settings",x+57,y+22,dark?0xFFFFFFU:0x253A54U,2);
            fb_draw_ui_text("Appearance and display",x+57,y+52,dark?0xBDD0E7U:0x5D728AU,1);
        }
        else if (tile == 4)
        {
            fb_draw_round_rect(x+20,y+17,24,34,7,0xD8E4F5U);
            fb_draw_circle(x+32,y+29,5,0x56749AU);
            fb_draw_ui_text("Lock screen",x+57,y+22,dark?0xFFFFFFU:0x253A54U,2);
            fb_draw_ui_text("Return to the lock page",x+57,y+52,dark?0xBDD0E7U:0x5D728AU,1);
        }
        else
        {
            fb_draw_circle(x+32,y+31,12,dark?0xDBE7F7U:0x637D9CU);
            fb_draw_ui_text("Close panel",x+57,y+22,dark?0xFFFFFFU:0x253A54U,2);
            fb_draw_ui_text("Return to Home",x+57,y+52,dark?0xBDD0E7U:0x5D728AU,1);
        }
    }
    fb_draw_ui_text("Swipe down from the top or press C / ESC to close", 275, RIO_H-52, dark?0xC3D4E8U:0x506883U,1);
    fb_present();
}

static int control_tile_hit(int px, int py)
{
    for (int tile=0;tile<6;++tile)
    {
        int x=0,y=0,w=0,h=0; control_tile_bounds(tile,&x,&y,&w,&h);
        if (px>=x && px<x+w && py>=y && py<y+h) return tile;
    }
    return -1;
}

static void control_action(int tile)
{
    if (tile == 1) (void)ui_settings_toggle_theme();
    else if (tile == 2) ui_open_app(UI_APP_WALLPAPER);
    else if (tile == 3) ui_open_app(UI_APP_SETTINGS);
    else if (tile == 4) { ui_state=0; active_app=-1; }
    else if (tile == 5) { ui_state=1; active_app=-1; }
}

static void render_ui()
{
    uint64_t epoch = rtc_get_epoch();
    if (ui_state == 0) ui_draw_lock_screen(epoch);
    else if (ui_state == 1) ui_draw_home_screen(selected_app, epoch);
    else if (ui_state == 2 && active_app >= 0) ui_apps_render(active_app, epoch);
    else if (ui_state == 3) draw_control_centre(epoch);
    dirty = 1;
    last_render_minute = epoch / 60ULL;
}

extern "C" int rio_init()
{
    if (initialized) return 0;
    command_stub_init();
    fs_init();
    int asset_result = rio_seed_assets();
    security_init();
    vfs_init();
    history_init();
    (void)env_set("HOME=/home/user");
    (void)env_set("USER=user");
    (void)env_set("SHELL=rioos");
    (void)env_set("PATH=/bin");
    (void)fd_set_process(0);
    if (fb_test_attach(framebuffer, RIO_W, RIO_H) != 0) return -2;
    ui_state = 0; selected_app = 0; active_app = -1; booting = 1;
    ui_draw_boot_screen();
    dirty = 1;
    initialized = 1;
    return asset_result;
}

extern "C" void rio_finish_boot()
{
    if (!initialized) return;
    booting = 0;
    ui_state = 0;
    render_ui();
}

extern "C" void rio_key(int key_code)
{
    if (!initialized || booting) return;
    char key = (char)key_code;
    if (ui_state == 0)
    {
        ui_state = 1; selected_app = 0;
        render_ui(); return;
    }
    if (ui_state == 1)
    {
        if (key == 'c' || key == 'C') ui_state = 3;
        else if (key == 'a' || key == 'A' || key == 'h' || key == 'H' || (unsigned char)key == 0x82U) { if (selected_app > 0) --selected_app; }
        else if (key == 'd' || key == 'D' || (unsigned char)key == 0x83U) { if (selected_app + 1 < UI_APP_COUNT) ++selected_app; }
        else if (key == 'w' || key == 'W' || (unsigned char)key == 0x80U) { if (selected_app >= 4) selected_app -= 4; }
        else if (key == 's' || key == 'S' || (unsigned char)key == 0x81U) { if (selected_app + 4 < UI_APP_COUNT) selected_app += 4; }
        else if (key == 'k' || key == 'K') ui_state = 0;
        else if (key >= '1' && key <= '7') ui_open_app(key - '1');
        else if (key == '\r' || key == '\n') ui_open_app(selected_app);
        render_ui(); return;
    }
    if (ui_state == 3)
    {
        if ((unsigned char)key == 27U || key == 'c' || key == 'C' || key == 'q' || key == 'Q') ui_state = 1;
        else if (key >= '1' && key <= '6') control_action(key - '1');
        render_ui(); return;
    }
    if (ui_state == 2 && active_app >= 0)
    {
        int result = ui_apps_key(active_app, key);
        if (result == 1) { ui_state = 1; active_app = -1; }
        else if (result == 2) ui_open_app(ui_apps_get_target(active_app));
        render_ui();
    }
}

extern "C" void rio_pointer(int x, int y, int pressed)
{
    if (!initialized || booting) return;
    if (x < 0) x=0; if (x>=RIO_W) x=RIO_W-1;
    if (y < 0) y=0; if (y>=RIO_H) y=RIO_H-1;
    if (pressed)
    {
        if (!pointer_down) { pointer_start_x=x; pointer_start_y=y; }
        pointer_down=1; return;
    }
    if (!pointer_down) return;
    pointer_down=0;
    int dy = y-pointer_start_y;
    if (ui_state == 0)
    {
        ui_state=1; selected_app=0; render_ui(); return;
    }
    if (ui_state == 1)
    {
        if ((pointer_start_y < 70 && dy > RIO_H/14) || y < 34)
        { ui_state=3; render_ui(); return; }
        const int cols=4, cell_w=(RIO_W-160)/cols;
        int hit=-1;
        for (int app=0;app<UI_APP_COUNT;++app)
        {
            int col=app%cols,row=app/cols;
            int cx=80+col*cell_w+cell_w/2, cy=row==0?368:500;
            int dx=x-cx, ddy=y-cy;
            if (dx>=-54 && dx<=54 && ddy>=-58 && ddy<=59) { hit=app; break; }
        }
        if (hit>=0) ui_open_app(hit);
        render_ui(); return;
    }
    if (ui_state == 3)
    {
        int tile=control_tile_hit(x,y);
        if (tile>=0) control_action(tile); else ui_state=1;
        render_ui(); return;
    }
    if (ui_state == 2 && active_app>=0)
    {
        if (y<36 && x<48) { ui_state=1; active_app=-1; }
        else
        {
            int result=ui_apps_pointer(active_app,x,y);
            if (result==1) { ui_state=1; active_app=-1; }
            else if (result==2) ui_open_app(ui_apps_get_target(active_app));
        }
        render_ui();
    }
}

extern "C" void rio_tick()
{
    if (!initialized || booting) return;
    uint64_t epoch=rtc_get_epoch(), minute=epoch/60ULL;
    if (minute != last_render_minute &&
        (ui_state==0 || ui_state==1 || (ui_state==2 && active_app==UI_APP_CLOCK) || ui_state==3))
        render_ui();
}
extern "C" uint32_t rio_framebuffer_ptr() { return (uint32_t)framebuffer; }
extern "C" int rio_framebuffer_width() { return RIO_W; }
extern "C" int rio_framebuffer_height() { return RIO_H; }
extern "C" int rio_take_dirty() { int value=dirty; dirty=0; return value; }
extern "C" int rio_get_ui_state() { return ui_state; }
extern "C" int rio_get_active_app() { return active_app; }
extern "C" int rio_get_seed_status() { return rio_seed_assets(); }

/* Small host-facing command API used by the standalone WASM32 terminal runner.
 * The command dispatcher and filesystem remain RioOS's real shared code. */
extern "C" uint32_t rio_cli_buffer_ptr()
{
    return (uint32_t)cli_command_buffer;
}

extern "C" uint32_t rio_cli_cwd_ptr()
{
    vfs_get_path(cli_cwd_buffer, sizeof(cli_cwd_buffer));
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
    /* The JS host handles exit so the WASM module cannot terminate its host. */
    execute_command(cli_command_buffer);
    uart_puts("\r\n");
    return 0;
}
