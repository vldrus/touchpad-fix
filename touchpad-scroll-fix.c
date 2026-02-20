/*
 * A simple fix for touchpad scroll speed in Wayland.
 *
 * MIT License.
 *
 * sudo apt-get install build-essential libinput-dev
 * cc -ldl -linput -shared -fPIC -o touchpad-scroll-fix.so touchpad-scroll-fix.c
 * sudo cp touchpad-scroll-fix.so /usr/local/lib/touchpad-scroll-fix.so
 * echo '0.5' | sudo tee /usr/local/etc/touchpad-scroll-fix.conf
 * echo '/usr/local/lib/touchpad-scroll-fix.so' | sudo tee -a /etc/ld.so.preload
 */

#define _GNU_SOURCE

#include <dlfcn.h>
#include <stdio.h>
#include <stddef.h>
#include <locale.h>

#include <libinput.h>

double libinput_event_pointer_get_scroll_value(
    struct libinput_event_pointer *event,
    enum libinput_pointer_axis axis)
{
    static const double DEFAULT_SCROLL_VALUE_MULTIPLIER = 1.0;
    static const char  *DEFAULT_CONFIG_FILE_PATH        = "/usr/local/etc/touchpad-scroll-fix.conf";

    double scroll_value_multiplier = DEFAULT_SCROLL_VALUE_MULTIPLIER;
    char *config_file_path = DEFAULT_CONFIG_FILE_PATH;

    locale_t c_locale = newlocale(LC_ALL_MASK, "C.UTF-8", NULL);
    locale_t prev_locale = uselocale(c_locale);

    FILE *config_file_stream = fopen(config_file_path, "r");

    if (config_file_stream != NULL) {
        fscanf(config_file_stream, "%lf", &scroll_value_multiplier);
        fclose(config_file_stream);
    }

    uselocale(prev_locale);
    freelocale(c_locale);

    double (*libinput_event_pointer_get_scroll_value_orig)(
        struct libinput_event_pointer *event,
        enum libinput_pointer_axis axis);

    libinput_event_pointer_get_scroll_value_orig =
        dlsym(RTLD_NEXT, "libinput_event_pointer_get_scroll_value");

    return libinput_event_pointer_get_scroll_value_orig(event, axis) * scroll_value_multiplier;
}
