#include "configator.h"

#include "config.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#ifndef DEBUG
	#define INI_LANG DATADIR "/lang/%s.ini"
	#define INI_CONFIG "/etc/ly/config.ini"
#else
	#define INI_LANG "../res/lang/%s.ini"
	#define INI_CONFIG "../res/config.ini"
#endif

static void lang_handle(void* data, char** pars, const int pars_count)
{
	if (*((char**)data) != NULL)
	{
		free (*((char**)data));
	}

	*((char**)data) = strdup(*pars);
}

static void config_handle_u8(void* data, char** pars, const int pars_count)
{
	if (strcmp(*pars, "") == 0)
	{
		*((u8*)data) = 0;
	}
	else
	{
		*((u8*)data) = atoi(*pars);
	}
}

static void config_handle_u16(void* data, char** pars, const int pars_count)
{
	if (strcmp(*pars, "") == 0)
	{
		*((u16*)data) = 0;
	}
	else
	{
		*((u16*)data) = atoi(*pars);
	}
}

void config_handle_str(void* data, char** pars, const int pars_count)
{
	if (*((char**)data) != NULL)
	{
		free(*((char**)data));
	}

	*((char**)data) = strdup(*pars);
}

static void config_handle_char(void* data, char** pars, const int pars_count)
{
	*((char*)data) = **pars;
}

static void config_handle_bool(void* data, char** pars, const int pars_count)
{
	*((bool*)data) = (strcmp("true", *pars) == 0);
}

void lang_load()
{
	// must be alphabetically sorted
	struct configator_param map_no_section[] =
	{
		{"capslock", &lang.capslock, lang_handle},
		{"err_alloc", &lang.err_alloc, lang_handle},
		{"err_bounds", &lang.err_bounds, lang_handle},
		{"err_chdir", &lang.err_chdir, lang_handle},
		{"err_console_dev", &lang.err_console_dev, lang_handle},
		{"err_dgn_oob", &lang.err_dgn_oob, lang_handle},
		{"err_domain", &lang.err_domain, lang_handle},
		{"err_hostname", &lang.err_hostname, lang_handle},
		{"err_mlock", &lang.err_mlock, lang_handle},
		{"err_null", &lang.err_null, lang_handle},
		{"err_pam", &lang.err_pam, lang_handle},
		{"err_pam_abort", &lang.err_pam_abort, lang_handle},
		{"err_pam_acct_expired", &lang.err_pam_acct_expired, lang_handle},
		{"err_pam_auth", &lang.err_pam_auth, lang_handle},
		{"err_pam_authinfo_unavail", &lang.err_pam_authinfo_unavail, lang_handle},
		{"err_pam_authok_reqd", &lang.err_pam_authok_reqd, lang_handle},
		{"err_pam_buf", &lang.err_pam_buf, lang_handle},
		{"err_pam_cred_err", &lang.err_pam_cred_err, lang_handle},
		{"err_pam_cred_expired", &lang.err_pam_cred_expired, lang_handle},
		{"err_pam_cred_insufficient", &lang.err_pam_cred_insufficient, lang_handle},
		{"err_pam_cred_unavail", &lang.err_pam_cred_unavail, lang_handle},
		{"err_pam_maxtries", &lang.err_pam_maxtries, lang_handle},
		{"err_pam_perm_denied", &lang.err_pam_perm_denied, lang_handle},
		{"err_pam_session", &lang.err_pam_session, lang_handle},
		{"err_pam_sys", &lang.err_pam_sys, lang_handle},
		{"err_pam_user_unknown", &lang.err_pam_user_unknown, lang_handle},
		{"err_path", &lang.err_path, lang_handle},
		{"err_perm_dir", &lang.err_perm_dir, lang_handle},
		{"err_perm_group", &lang.err_perm_group, lang_handle},
		{"err_perm_user", &lang.err_perm_user, lang_handle},
		{"err_pwnam", &lang.err_pwnam, lang_handle},
		{"err_user_gid", &lang.err_user_gid, lang_handle},
		{"err_user_init", &lang.err_user_init, lang_handle},
		{"err_user_uid", &lang.err_user_uid, lang_handle},
		{"err_xsessions_dir", &lang.err_xsessions_dir, lang_handle},
		{"err_xsessions_open", &lang.err_xsessions_open, lang_handle},
		{"f1", &lang.f1, lang_handle},
		{"f2", &lang.f2, lang_handle},
		{"login", &lang.login, lang_handle},
		{"logout", &lang.logout, lang_handle},
		{"numlock", &lang.numlock, lang_handle},
		{"password", &lang.password, lang_handle},
		{"shell", &lang.shell, lang_handle},
		{"wayland", &lang.wayland, lang_handle},
		{"xinitrc", &lang.xinitrc, lang_handle},
	};

	uint16_t map_len[] = {45};
	struct configator_param* map[] =
	{
		map_no_section,
	};

	uint16_t sections_len = 0;
	struct configator_param* sections = NULL;

	struct configator lang;
	lang.map = map;
	lang.map_len = map_len;
	lang.sections = sections;
	lang.sections_len = sections_len;

	char file[256];
	snprintf(file, 256, INI_LANG, config.lang);

	if (access(file, F_OK) != -1)
	{
		configator(&lang, file);
	}
}

void config_load(const char *cfg_path)
{
	if (cfg_path == NULL)
	{
		cfg_path = INI_CONFIG;
	}
	// must be alphabetically sorted
	struct configator_param map_no_section[] =
	{
		{"animate", &config.animate, config_handle_bool},
		{"animation", &config.animation, config_handle_u8},
		{"asterisk", &config.asterisk, config_handle_char},
		{"bg", &config.bg, config_handle_u8},
		{"blank_box", &config.blank_box, config_handle_bool},
		{"blank_password", &config.blank_password, config_handle_bool},
		{"console_dev", &config.console_dev, config_handle_str},
		{"default_input", &config.default_input, config_handle_u8},
		{"fg", &config.fg, config_handle_u8},
		{"hide_borders", &config.hide_borders, config_handle_bool},
		{"hide_f1_commands", &config.hide_f1_commands, config_handle_bool},
		{"input_len", &config.input_len, config_handle_u8},
		{"lang", &config.lang, config_handle_str},
		{"load", &config.load, config_handle_bool},
		{"margin_box_h", &config.margin_box_h, config_handle_u8},
		{"margin_box_v", &config.margin_box_v, config_handle_u8},
		{"max_desktop_len", &config.max_desktop_len, config_handle_u8},
		{"max_login_len", &config.max_login_len, config_handle_u8},
		{"max_password_len", &config.max_password_len, config_handle_u8},
		{"mcookie_cmd", &config.mcookie_cmd, config_handle_str},
		{"min_refresh_delta", &config.min_refresh_delta, config_handle_u16},
		{"path", &config.path, config_handle_str},
		{"restart_cmd", &config.restart_cmd, config_handle_str},
		{"save", &config.save, config_handle_bool},
		{"save_file", &config.save_file, config_handle_str},
		{"service_name", &config.service_name, config_handle_str},
		{"shutdown_cmd", &config.shutdown_cmd, config_handle_str},
		{"term_reset_cmd", &config.term_reset_cmd, config_handle_str},
		{"tty", &config.tty, config_handle_u8},
		{"wayland_cmd", &config.wayland_cmd, config_handle_str},
		{"wayland_specifier", &config.wayland_specifier, config_handle_bool},
		{"waylandsessions", &config.waylandsessions, config_handle_str},
		{"x_cmd", &config.x_cmd, config_handle_str},
		{"x_cmd_setup", &config.x_cmd_setup, config_handle_str},
		{"xauth_cmd", &config.xauth_cmd, config_handle_str},
		{"xsessions", &config.xsessions, config_handle_str},
	};

	uint16_t map_len[] = {34};
	struct configator_param* map[] =
	{
		map_no_section,
	};

	uint16_t sections_len = 0;
	struct configator_param* sections = NULL;

	struct configator config;
	config.map = map;
	config.map_len = map_len;
	config.sections = sections;
	config.sections_len = sections_len;

	configator(&config, (char *) cfg_path);
}

void lang_defaults()
{
	lang.capslock = strdup("capslock");
	lang.err_alloc = strdup("failed memory allocation");
	lang.err_bounds = strdup("out-of-bounds index");
	lang.err_chdir = strdup("failed to open home folder");
	lang.err_console_dev = strdup("failed to access console");
	lang.err_dgn_oob = strdup("log message");
	lang.err_domain = strdup("invalid domain");
	lang.err_hostname = strdup("failed to get hostname");
	lang.err_mlock = strdup("failed to lock password memory");
	lang.err_null = strdup("null pointer");
	lang.err_pam = strdup("pam transaction failed");
	lang.err_pam_abort = strdup("pam transaction aborted");
	lang.err_pam_acct_expired = strdup("account expired");
	lang.err_pam_auth = strdup("authentication error");
	lang.err_pam_authinfo_unavail = strdup("failed to get user info");
	lang.err_pam_authok_reqd = strdup("token expired");
	lang.err_pam_buf = strdup("memory buffer error");
	lang.err_pam_cred_err = strdup("failed to set credentials");
	lang.err_pam_cred_expired = strdup("credentials expired");
	lang.err_pam_cred_insufficient = strdup("insufficient credentials");
	lang.err_pam_cred_unavail = strdup("failed to get credentials");
	lang.err_pam_maxtries = strdup("reached maximum tries limit");
	lang.err_pam_perm_denied = strdup("permission denied");
	lang.err_pam_session = strdup("session error");
	lang.err_pam_sys = strdup("system error");
	lang.err_pam_user_unknown = strdup("unknown user");
	lang.err_path = strdup("failed to set path");
	lang.err_perm_dir = strdup("failed to change current directory");
	lang.err_perm_group = strdup("failed to downgrade group permissions");
	lang.err_perm_user = strdup("failed to downgrade user permissions");
	lang.err_pwnam = strdup("failed to get user info");
	lang.err_user_gid = strdup("failed to set user GID");
	lang.err_user_init = strdup("failed to initialize user");
	lang.err_user_uid = strdup("failed to set user UID");
	lang.err_xsessions_dir = strdup("failed to find sessions folder");
	lang.err_xsessions_open = strdup("failed to open sessions folder");
	lang.f1 = strdup("F1 shutdown");
	lang.f2 = strdup("F2 reboot");
	lang.login = strdup("login:");
	lang.logout = strdup("logged out");
	lang.numlock = strdup("numlock");
	lang.password = strdup("password:");
	lang.shell = strdup("shell");
	lang.wayland = strdup("wayland");
	lang.xinitrc = strdup("xinitrc");
}

void config_defaults()
{
	config.animate = false;
	config.animation = 0;
	config.asterisk = '*';
	config.bg = 0;
	config.blank_box = true;
	config.blank_password = false;
	config.console_dev = strdup("/dev/console");
	config.default_input = PASSWORD_INPUT;
	config.fg = 9;
	config.hide_borders = false;
	config.input_len = 34;
	config.lang = strdup("en");
	config.load = true;
	config.margin_box_h = 2;
	config.margin_box_v = 1;
	config.max_desktop_len = 100;
	config.max_login_len = 255;
	config.max_password_len = 255;
	config.mcookie_cmd = strdup("/usr/bin/mcookie");
	config.min_refresh_delta = 5;
	config.path = strdup("/sbin:/bin:/usr/local/sbin:/usr/local/bin:/usr/bin:/usr/sbin");
	config.restart_cmd = strdup("/sbin/shutdown -r now");
	config.save = true;
	config.save_file = strdup("/etc/ly/save");
	config.service_name = strdup("ly");
	config.shutdown_cmd = strdup("/sbin/shutdown -a now");
	config.term_reset_cmd = strdup("/usr/bin/tput reset");
	config.tty = 2;
	config.wayland_cmd = strdup(DATADIR "/wsetup.sh");
	config.wayland_specifier = false;
	config.waylandsessions = strdup("/usr/share/wayland-sessions");
	config.x_cmd = strdup("/usr/bin/X");
	config.x_cmd_setup = strdup(DATADIR "/xsetup.sh");
	config.xauth_cmd = strdup("/usr/bin/xauth");
	config.xsessions = strdup("/usr/share/xsessions");
}

void lang_free()
{
	free(lang.capslock);
	free(lang.err_alloc);
	free(lang.err_bounds);
	free(lang.err_chdir);
	free(lang.err_console_dev);
	free(lang.err_dgn_oob);
	free(lang.err_domain);
	free(lang.err_hostname);
	free(lang.err_mlock);
	free(lang.err_null);
	free(lang.err_pam);
	free(lang.err_pam_abort);
	free(lang.err_pam_acct_expired);
	free(lang.err_pam_auth);
	free(lang.err_pam_authinfo_unavail);
	free(lang.err_pam_authok_reqd);
	free(lang.err_pam_buf);
	free(lang.err_pam_cred_err);
	free(lang.err_pam_cred_expired);
	free(lang.err_pam_cred_insufficient);
	free(lang.err_pam_cred_unavail);
	free(lang.err_pam_maxtries);
	free(lang.err_pam_perm_denied);
	free(lang.err_pam_session);
	free(lang.err_pam_sys);
	free(lang.err_pam_user_unknown);
	free(lang.err_path);
	free(lang.err_perm_dir);
	free(lang.err_perm_group);
	free(lang.err_perm_user);
	free(lang.err_pwnam);
	free(lang.err_user_gid);
	free(lang.err_user_init);
	free(lang.err_user_uid);
	free(lang.err_xsessions_dir);
	free(lang.err_xsessions_open);
	free(lang.f1);
	free(lang.f2);
	free(lang.login);
	free(lang.logout);
	free(lang.numlock);
	free(lang.password);
	free(lang.shell);
	free(lang.wayland);
	free(lang.xinitrc);
}

void config_free()
{
	free(config.console_dev);
	free(config.lang);
	free(config.mcookie_cmd);
	free(config.path);
	free(config.restart_cmd);
	free(config.save_file);
	free(config.service_name);
	free(config.shutdown_cmd);
	free(config.term_reset_cmd);
	free(config.wayland_cmd);
	free(config.waylandsessions);
	free(config.x_cmd);
	free(config.x_cmd_setup);
	free(config.xauth_cmd);
	free(config.xsessions);
}
