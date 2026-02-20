/*
 * A simple fix for touchpad scroll speed in Wayland
 *
 * sudo apt-get install build-essential libinput-dev
 * cc -std=gnu89 -ldl -shared -fPIC -o touchpad-fix.so touchpad-fix.c
 * sudo cp touchpad-fix.so /usr/local/lib/touchpad-fix.so
 * echo '0.5' | sudo tee /usr/local/etc/touchpad-fix.conf
 * echo '/usr/local/lib/touchpad-fix.so' | sudo tee -a /etc/ld.so.preload
 */

#define _GNU_SOURCE

#include <dlfcn.h>
#include <stdio.h>
#include <stddef.h>
#include <locale.h>

#include <libinput.h>

const double DEFAULT_SCROLL_VALUE_MULTIPLIER = 1.0;
const char  *DEFAULT_CONFIG_FILE_PATH        = "/usr/local/etc/touchpad-fix.conf";

typedef double (*libinput_event_pointer_get_scroll_value_orig_t)
(
    struct libinput_event_pointer *event,
    enum libinput_pointer_axis axis
);

double libinput_event_pointer_get_scroll_value
(
    struct libinput_event_pointer *event,
    enum libinput_pointer_axis axis
)
{
    locale_t c_locale = newlocale(LC_NUMERIC_MASK, "C", NULL);
    locale_t prev_locale = uselocale(c_locale);

    double scroll_value_multiplier = DEFAULT_SCROLL_VALUE_MULTIPLIER;

    FILE *config_file_stream = fopen(DEFAULT_CONFIG_FILE_PATH, "r");

    if (config_file_stream) {
        fscanf(config_file_stream, "%lf", &scroll_value_multiplier);
        fclose(config_file_stream);
    }

    uselocale(prev_locale);
    freelocale(c_locale);

    libinput_event_pointer_get_scroll_value_orig_t
        libinput_event_pointer_get_scroll_value_orig =
            dlsym(RTLD_NEXT, "libinput_event_pointer_get_scroll_value");

    return libinput_event_pointer_get_scroll_value_orig(event, axis) * scroll_value_multiplier;
}
