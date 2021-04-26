#ifndef H_LY_CONFIG
#define H_LY_CONFIG

#include "ctypes.h"

enum INPUTS {
	SESSION_SWITCH,
	LOGIN_INPUT,
	PASSWORD_INPUT,
};

struct lang
{
	char* capslock;
	char* err_alloc;
	char* err_bounds;
	char* err_chdir;
	char* err_console_dev;
	char* err_dgn_oob;
	char* err_domain;
	char* err_hostname;
	char* err_mlock;
	char* err_null;
	char* err_pam;
	char* err_pam_abort;
	char* err_pam_acct_expired;
	char* err_pam_auth;
	char* err_pam_authinfo_unavail;
	char* err_pam_authok_reqd;
	char* err_pam_buf;
	char* err_pam_cred_err;
	char* err_pam_cred_expired;
	char* err_pam_cred_insufficient;
	char* err_pam_cred_unavail;
	char* err_pam_maxtries;
	char* err_pam_perm_denied;
	char* err_pam_session;
	char* err_pam_sys;
	char* err_pam_user_unknown;
	char* err_path;
	char* err_perm_dir;
	char* err_perm_group;
	char* err_perm_user;
	char* err_pwnam;
	char* err_user_gid;
	char* err_user_init;
	char* err_user_uid;
	char* err_xsessions_dir;
	char* err_xsessions_open;
	char* f1;
	char* f2;
	char* login;
	char* logout;
	char* numlock;
	char* password;
	char* shell;
	char* wayland;
	char* xinitrc;
};

struct config
{
	bool animate;
	u8 animation;
	char asterisk;
        bool asterisk_empty;
	u8 bg;
	bool blank_box;
	bool blank_password;
	char* console_dev;
	u8 default_input;
	u8 fg;
	bool hide_borders;
	u8 input_len;
	char* lang;
	bool load;
	u8 margin_box_h;
	u8 margin_box_v;
	u8 max_desktop_len;
	u8 max_login_len;
	u8 max_password_len;
	char* mcookie_cmd;
	u16 min_refresh_delta;
	char* path;
	char* restart_cmd;
	bool save;
	char* save_file;
	char* service_name;
	char* shutdown_cmd;
	char* term_reset_cmd;
	u8 tty;
	char* wayland_cmd;
	bool wayland_specifier;
	char* waylandsessions;
	char* x_cmd;
	char* x_cmd_setup;
	char* xauth_cmd;
	char* xsessions;
};

extern struct lang lang;
extern struct config config;

void config_handle_str(void* data, char** pars, const int pars_count);
void lang_load();
void config_load(const char *cfg_path);
void lang_defaults();
void config_defaults();
void lang_free();
void config_free();

#endif
