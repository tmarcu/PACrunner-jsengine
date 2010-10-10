/*
 *
 *  PACrunner - Proxy configuration daemon
 *
 *  Copyright (C) 2010  Intel Corporation. All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <dbus/dbus.h>
#include <glib.h>

#define PACRUNNER_SERVICE	"org.pacrunner"
#define PACRUNNER_PATH		"/org/pacrunner"

#define PACRUNNER_ERROR_INTERFACE	PACRUNNER_SERVICE ".Error"

#define PACRUNNER_MANAGER_INTERFACE	PACRUNNER_SERVICE ".Manager"
#define PACRUNNER_MANAGER_PATH		PACRUNNER_PATH "/manager"

#define PACRUNNER_CLIENT_INTERFACE	PACRUNNER_SERVICE ".Client"
#define PACRUNNER_CLIENT_PATH		PACRUNNER_PATH "/client"


void pacrunner_info(const char *format, ...)
				__attribute__((format(printf, 1, 2)));
void pacrunner_warn(const char *format, ...)
				__attribute__((format(printf, 1, 2)));
void pacrunner_error(const char *format, ...)
				__attribute__((format(printf, 1, 2)));
void pacrunner_debug(const char *format, ...)
				__attribute__((format(printf, 1, 2)));

struct pacrunner_debug_desc {
	const char *name;
	const char *file;
#define PACRUNNER_DEBUG_FLAG_DEFAULT (0)
#define PACRUNNER_DEBUG_FLAG_PRINT   (1 << 0)
	unsigned int flags;
} __attribute__((aligned(8)));

#define DBG(fmt, arg...) do { \
	static struct pacrunner_debug_desc __pacrunner_debug_desc \
	__attribute__((used, section("__debug"), aligned(8))) = { \
		.file = __FILE__, .flags = PACRUNNER_DEBUG_FLAG_DEFAULT, \
	}; \
	if (__pacrunner_debug_desc.flags & PACRUNNER_DEBUG_FLAG_PRINT) \
		pacrunner_debug("%s:%s() " fmt, \
					__FILE__, __FUNCTION__ , ## arg); \
} while (0)

int __pacrunner_log_init(const char *debug, gboolean detach);
void __pacrunner_log_cleanup(void);


typedef void (* __pacrunner_download_cb) (char *content, void *user_data);

int __pacrunner_download_init(void);
void __pacrunner_download_cleanup(void);
int __pacrunner_download_update(const char *interface, const char *url,
			__pacrunner_download_cb callback, void *user_data);

int __pacrunner_manager_init(DBusConnection *conn);
void __pacrunner_manager_cleanup();

int __pacrunner_client_init(DBusConnection *conn);
void __pacrunner_client_cleanup();

int __pacrunner_mozjs_init(void);
void __pacrunner_mozjs_cleanup(void);
int __pacrunner_mozjs_set_server(const char *interface, const char *server);
int __pacrunner_mozjs_set_script(const char *interface, const char *script);
void __pacrunner_mozjs_clear(void);
const char *__pacrunner_mozjs_execute(const char *url, const char *host);
