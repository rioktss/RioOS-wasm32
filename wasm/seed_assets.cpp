#include "fs.h"
#include "string.h"
#include "types.h"

#define DECLARE_ASSET(symbol) \
    extern "C" const uint8_t symbol##_start[]; \
    extern "C" const uint8_t symbol##_end[]
#define ASSET_SIZE(symbol) ((uint64_t)(symbol##_end - symbol##_start))

DECLARE_ASSET(rio_asset_apps_terminal_rpk);
DECLARE_ASSET(rio_asset_apps_calculator_rpk);
DECLARE_ASSET(rio_asset_apps_clock_rpk);
DECLARE_ASSET(rio_asset_apps_files_rpk);
DECLARE_ASSET(rio_asset_apps_launcher_rpk);
DECLARE_ASSET(rio_asset_apps_wallpaper_rpk);
DECLARE_ASSET(rio_asset_apps_pengaturan_rpk);
DECLARE_ASSET(rio_asset_assets_icons_terminal_bmp);
DECLARE_ASSET(rio_asset_assets_icons_calculator_bmp);
DECLARE_ASSET(rio_asset_assets_icons_clock_bmp);
DECLARE_ASSET(rio_asset_assets_icons_files_bmp);
DECLARE_ASSET(rio_asset_assets_icons_launcher_bmp);
DECLARE_ASSET(rio_asset_assets_icons_wallpaper_bmp);
DECLARE_ASSET(rio_asset_assets_icons_settings_bmp);
DECLARE_ASSET(rio_asset_assets_wallpapers_rioos_default_bmp);

static void mkdir_if_missing(const char* path)
{
    int id = fs_resolve(path, fs_get_root());
    if (id < 0) (void)fs_mkdir_p(path, fs_get_root());
}

static int seed_file_if_missing(const char* path, const uint8_t* data, uint64_t length)
{
    if (!path || !data || length == 0 || length > 1024ULL * 1024ULL) return -1;
    if (fs_resolve(path, fs_get_root()) >= 0) return 0;
    return fs_write_data(path, fs_get_root(), data, length);
}

extern "C" int rio_seed_assets()
{
    fs_set_credentials(0U, 0U);
    const char* dirs[] = {"/apps", "/icons", "/wallpapers", "/settings", "/home", "/home/user", "/storage", "/storage/home"};
    for (unsigned i = 0; i < sizeof(dirs) / sizeof(dirs[0]); ++i) mkdir_if_missing(dirs[i]);

    int errors = 0;
#define SEED(path, symbol) do { if (seed_file_if_missing(path, symbol##_start, ASSET_SIZE(symbol)) != 0) ++errors; } while (0)
    SEED("/apps/terminal.rpk", rio_asset_apps_terminal_rpk);
    SEED("/apps/calculator.rpk", rio_asset_apps_calculator_rpk);
    SEED("/apps/clock.rpk", rio_asset_apps_clock_rpk);
    SEED("/apps/files.rpk", rio_asset_apps_files_rpk);
    SEED("/apps/launcher.rpk", rio_asset_apps_launcher_rpk);
    SEED("/apps/wallpaper.rpk", rio_asset_apps_wallpaper_rpk);
    SEED("/apps/pengaturan.rpk", rio_asset_apps_pengaturan_rpk);
    SEED("/icons/terminal.bmp", rio_asset_assets_icons_terminal_bmp);
    SEED("/icons/calculator.bmp", rio_asset_assets_icons_calculator_bmp);
    SEED("/icons/clock.bmp", rio_asset_assets_icons_clock_bmp);
    SEED("/icons/files.bmp", rio_asset_assets_icons_files_bmp);
    SEED("/icons/launcher.bmp", rio_asset_assets_icons_launcher_bmp);
    SEED("/icons/wallpaper.bmp", rio_asset_assets_icons_wallpaper_bmp);
    SEED("/icons/settings.bmp", rio_asset_assets_icons_settings_bmp);
    SEED("/wallpapers/rioos-default.bmp", rio_asset_assets_wallpapers_rioos_default_bmp);
#undef SEED

    static const uint8_t dark_theme[] = {'d','a','r','k'};
    static const uint8_t default_wallpaper[] = "/wallpapers/rioos-default.bmp";
    if (fs_resolve("/settings/theme.cfg", fs_get_root()) < 0 &&
        fs_write_data("/settings/theme.cfg", fs_get_root(), dark_theme, sizeof(dark_theme)) != 0) ++errors;
    if (fs_resolve("/settings/wallpaper.cfg", fs_get_root()) < 0 &&
        fs_write_data("/settings/wallpaper.cfg", fs_get_root(), default_wallpaper, sizeof(default_wallpaper) - 1ULL) != 0) ++errors;

    int settings_dir = fs_resolve("/settings", fs_get_root());
    if (settings_dir >= 0) (void)fs_set_owner(settings_dir, 1000U, 1000U);
    (void)fs_set_owner_path("/settings/theme.cfg", fs_get_root(), 1000U, 1000U);
    (void)fs_set_owner_path("/settings/wallpaper.cfg", fs_get_root(), 1000U, 1000U);
    return errors == 0 ? 0 : -errors;
}
