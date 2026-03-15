/*
 * A simple fix for touchpad scroll speed in Wayland.
 */

/*
 * Copyright (c) 2026 Vlad
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*
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
    const char *config_file_path = DEFAULT_CONFIG_FILE_PATH;

    locale_t c_locale = newlocale(LC_ALL_MASK, "C", NULL);
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
