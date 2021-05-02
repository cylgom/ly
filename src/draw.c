#include "dragonfail.h"
#include "termbox.h"
#include "ctypes.h"

#include "inputs.h"
#include "utils.h"
#include "config.h"
#include "draw.h"

#include <ctype.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>

#if defined(__DragonFly__) || defined(__FreeBSD__)
	#include <sys/kbio.h>
#else // linux
	#include <linux/kd.h>
#endif

#define DOOM_STEPS 13

void draw_init(struct term_buf* buf)
{
	buf->width = tb_width();
	buf->height = tb_height();
	hostname(&buf->info_line);

	u16 len_login = strlen(lang.login);
	u16 len_password = strlen(lang.password);

	if (len_login > len_password)
	{
		buf->labels_max_len = len_login;
	}
	else
	{
		buf->labels_max_len = len_password;
	}

	buf->box_height = 7 + (2 * config.margin_box_v);
	buf->box_width =
		(2 * config.margin_box_h)
		+ (config.input_len + 1)
		+ buf->labels_max_len;

#if defined(__linux__)
	buf->box_chars.left_up = 0x250c;
	buf->box_chars.left_down = 0x2514;
	buf->box_chars.right_up = 0x2510;
	buf->box_chars.right_down = 0x2518;
	buf->box_chars.top = 0x2500;
	buf->box_chars.bot = 0x2500;
	buf->box_chars.left = 0x2502;
	buf->box_chars.right = 0x2502;
#else
	buf->box_chars.left_up = '+';
	buf->box_chars.left_down = '+';
	buf->box_chars.right_up = '+';
	buf->box_chars.right_down= '+';
	buf->box_chars.top = '-';
	buf->box_chars.bot = '-';
	buf->box_chars.left = '|';
	buf->box_chars.right = '|';
#endif
}

static void doom_free(struct term_buf* buf);
static void matrix_free(struct term_buf* buf);

void draw_free(struct term_buf* buf)
{
	if (config.animate)
	{
		switch (config.animation)
		{
			case 0:
				doom_free(buf);
				break;
			case 1:
				matrix_free(buf);
				break;
		}
	}
}

void draw_box(struct term_buf* buf)
{
	u16 box_x = (buf->width - buf->box_width) / 2;
	u16 box_y = (buf->height - buf->box_height) / 2;
	u16 box_x2 = (buf->width + buf->box_width) / 2;
	u16 box_y2 = (buf->height + buf->box_height) / 2;
	buf->box_x = box_x;
	buf->box_y = box_y;

	if (!config.hide_borders)
	{
		// corners
		tb_change_cell(
			box_x - 1,
			box_y - 1,
			buf->box_chars.left_up,
			config.fg,
			config.bg);
		tb_change_cell(
			box_x2,
			box_y - 1,
			buf->box_chars.right_up,
			config.fg,
			config.bg);
		tb_change_cell(
			box_x - 1,
			box_y2,
			buf->box_chars.left_down,
			config.fg,
			config.bg);
		tb_change_cell(
			box_x2,
			box_y2,
			buf->box_chars.right_down,
			config.fg,
			config.bg);

		// top and bottom
		struct tb_cell c1 = {buf->box_chars.top, config.fg, config.bg};
		struct tb_cell c2 = {buf->box_chars.bot, config.fg, config.bg};

		for (u8 i = 0; i < buf->box_width; ++i)
		{
			tb_put_cell(
				box_x + i,
				box_y - 1,
				&c1);
			tb_put_cell(
				box_x + i,
				box_y2,
				&c2);
		}

		// left and right
		c1.ch = buf->box_chars.left;
		c2.ch = buf->box_chars.right;

		for (u8 i = 0; i < buf->box_height; ++i)
		{
			tb_put_cell(
				box_x - 1,
				box_y + i,
				&c1);

			tb_put_cell(
				box_x2,
				box_y + i,
				&c2);
		}
	}

	if (config.blank_box)
	{
		struct tb_cell blank = {' ', config.fg, config.bg};

		for (u8 i = 0; i < buf->box_height; ++i)
		{
			for (u8 k = 0; k < buf->box_width; ++k)
			{
				tb_put_cell(
					box_x + k,
					box_y + i,
					&blank);
			}
		}
	}
}

struct tb_cell* strn_cell(char* s, u16 len) // throws
{
	struct tb_cell* cells = malloc((sizeof (struct tb_cell)) * len);
	char* s2 = s;
	u32 c;

	if (cells != NULL)
	{
		for (u16 i = 0; i < len; ++i)
		{
			if ((s2 - s) >= len)
			{
				break;
			}

			s2 += utf8_char_to_unicode(&c, s2);

			cells[i].ch = c;
			cells[i].bg = config.bg;
			cells[i].fg = config.fg;
		}
	}
	else
	{
		dgn_throw(DGN_ALLOC);
	}

	return cells;
}

struct tb_cell* str_cell(char* s) // throws
{
	return strn_cell(s, strlen(s));
}

void draw_labels(struct term_buf* buf) // throws
{
	// login text
	struct tb_cell* login = str_cell(lang.login);

	if (dgn_catch())
	{
		dgn_reset();
	}
	else
	{
		tb_blit(
			buf->box_x + config.margin_box_h,
			buf->box_y + config.margin_box_v + 4,
			strlen(lang.login),
			1,
			login);
		free(login);
	}

	// password text
	struct tb_cell* password = str_cell(lang.password);

	if (dgn_catch())
	{
		dgn_reset();
	}
	else
	{
		tb_blit(
			buf->box_x + config.margin_box_h,
			buf->box_y + config.margin_box_v + 6,
			strlen(lang.password),
			1,
			password);
		free(password);
	}

	if (buf->info_line != NULL)
	{
		u16 len = strlen(buf->info_line);
		struct tb_cell* info_cell = str_cell(buf->info_line);

		if (dgn_catch())
		{
			dgn_reset();
		}
		else
		{
			tb_blit(
				buf->box_x + ((buf->box_width - len) / 2),
				buf->box_y + config.margin_box_v,
				len,
				1,
				info_cell);
			free(info_cell);
		}
	}
}

void draw_f_commands()
{
	struct tb_cell* f1 = str_cell(lang.f1);

	if (dgn_catch())
	{
		dgn_reset();
	}
	else
	{
		tb_blit(0, 0, strlen(lang.f1), 1, f1);
		free(f1);
	}

	struct tb_cell* f2 = str_cell(lang.f2);

	if (dgn_catch())
	{
		dgn_reset();
	}
	else
	{
		tb_blit(strlen(lang.f1) + 1, 0, strlen(lang.f2), 1, f2);
		free(f2);
	}
}

void draw_lock_state(struct term_buf* buf)
{
	// get values
	int fd = open(config.console_dev, O_RDONLY);

	if (fd < 0)
	{
		buf->info_line = lang.err_console_dev;
		return;
	}

	bool numlock_on;
	bool capslock_on;

#if defined(__DragonFly__) || defined(__FreeBSD__)
	int led;
	ioctl(fd, KDGETLED, &led);
	numlock_on = led & LED_NUM;
	capslock_on = led & LED_CAP;
#else // linux
	char led;
	ioctl(fd, KDGKBLED, &led);
	numlock_on = led & K_NUMLOCK;
	capslock_on = led & K_CAPSLOCK;
#endif

	close(fd);

	// print text
	u16 pos_x = buf->width - strlen(lang.numlock);

	if (numlock_on)
	{
		struct tb_cell* numlock = str_cell(lang.numlock);

		if (dgn_catch())
		{
			dgn_reset();
		}
		else
		{
			tb_blit(pos_x, 0, strlen(lang.numlock), 1, numlock);
			free(numlock);
		}
	}

	pos_x -= strlen(lang.capslock) + 1;

	if (capslock_on)
	{
		struct tb_cell* capslock = str_cell(lang.capslock);

		if (dgn_catch())
		{
			dgn_reset();
		}
		else
		{
			tb_blit(pos_x, 0, strlen(lang.capslock), 1, capslock);
			free(capslock);
		}
	}
}

void draw_desktop(struct desktop* target)
{
	u16 len = strlen(target->list[target->cur]);

	if (len > (target->visible_len - 3))
	{
		len = target->visible_len - 3;
	}

	tb_change_cell(
		target->x,
		target->y,
		'<',
		config.fg,
		config.bg);

	tb_change_cell(
		target->x + target->visible_len - 1,
		target->y,
		'>',
		config.fg,
		config.bg);

	for (u16 i = 0; i < len; ++ i)
	{
		tb_change_cell(
			target->x + i + 2,
			target->y,
			target->list[target->cur][i],
			config.fg,
			config.bg);
	}
}

void draw_input(struct text* input)
{
	u16 len = strlen(input->text);
	u16 visible_len = input->visible_len;

	if (len > visible_len)
	{
		len = visible_len;
	}

	struct tb_cell* cells = strn_cell(input->visible_start, len);

	if (dgn_catch())
	{
		dgn_reset();
	}
	else
	{
		tb_blit(input->x, input->y, len, 1, cells);
		free(cells);

		struct tb_cell c1 = {' ', config.fg, config.bg};

		for (u16 i = input->end - input->visible_start; i < visible_len; ++i)
		{
			tb_put_cell(
				input->x + i,
				input->y,
				&c1);
		}
	}
}

void draw_input_mask(struct text* input)
{
	u16 len = strlen(input->text);
	u16 visible_len = input->visible_len;

	if (len > visible_len)
	{
		len = visible_len;
	}

	struct tb_cell c1 = {config.asterisk, config.fg, config.bg};
	struct tb_cell c2 = {' ', config.fg, config.bg};

	for (u16 i = 0; i < visible_len; ++i)
	{
		if (input->visible_start + i < input->end)
		{
			tb_put_cell(
				input->x + i,
				input->y,
				&c1);
		}
		else
		{
			tb_put_cell(
				input->x + i,
				input->y,
				&c2);
		}
	}
}

void position_input(
	struct term_buf* buf,
	struct desktop* desktop,
	struct text* login,
	struct text* password)
{
	u16 x = buf->box_x + config.margin_box_h + buf->labels_max_len + 1;
	i32 len = buf->box_x + buf->box_width - config.margin_box_h - x;

	if (len < 0)
	{
		return;
	}

	desktop->x = x;
	desktop->y = buf->box_y + config.margin_box_v + 2;
	desktop->visible_len = len;

	login->x = x;
	login->y = buf->box_y + config.margin_box_v + 4;
	login->visible_len = len;

	password->x = x;
	password->y = buf->box_y + config.margin_box_v + 6;
	password->visible_len = len;
}

static void doom_init(struct term_buf* buf)
{
	buf->init_width = buf->width;
	buf->init_height = buf->height;
	buf->astate.doom = malloc(sizeof(struct doom_state));

	if (buf->astate.doom == NULL)
	{
		dgn_throw(DGN_ALLOC);
	}

	u16 tmp_len = buf->width * buf->height;
	buf->astate.doom->buf = malloc(tmp_len);
	tmp_len -= buf->width;

	if (buf->astate.doom->buf == NULL)
	{
		dgn_throw(DGN_ALLOC);
	}

	memset(buf->astate.doom->buf, 0, tmp_len);
	memset(buf->astate.doom->buf + tmp_len, DOOM_STEPS - 1, buf->width);
}

static void doom_free(struct term_buf* buf)
{
	free(buf->astate.doom->buf);
	free(buf->astate.doom);
}

// Adapted from cmatrix
static void matrix_init(struct term_buf* buf)
{
	buf->init_width = buf->width;
	buf->init_height = buf->height;
	buf->astate.matrix = malloc(sizeof(struct matrix_state));
	struct matrix_state* s = buf->astate.matrix;

	if (s == NULL)
	{
		dgn_throw(DGN_ALLOC);
	}

	u16 len = buf->height + 1;
	s->grid = malloc(sizeof(struct matrix_dot*) * len);

	if (s->grid == NULL)
	{
		dgn_throw(DGN_ALLOC);
	}

	len = (buf->height + 1) * buf->width;
	(s->grid)[0] = malloc(sizeof(struct matrix_dot) * len);

	if ((s->grid)[0] == NULL)
	{
		dgn_throw(DGN_ALLOC);
	}

	for (int i = 1; i <= buf->height; ++i)
	{
		s->grid[i] = s->grid[i - 1] + buf->width;

		if (s->grid[i] == NULL)
		{
			dgn_throw(DGN_ALLOC);
		}
	}

	s->length = malloc(buf->width * sizeof(int));

	if (s->length == NULL)
	{
		dgn_throw(DGN_ALLOC);
	}

	s->spaces = malloc(buf->width * sizeof(int));

	if (s->spaces == NULL)
	{
		dgn_throw(DGN_ALLOC);
	}

	s->updates = malloc(buf->width * sizeof(int));

	if (s->updates == NULL)
	{
		dgn_throw(DGN_ALLOC);
	}

	// Initialize grid
	for (int i = 0; i <= buf->height; ++i)
	{
		for (int j = 0; j <= buf->width - 1; j += 2)
		{
			s->grid[i][j].val = -1;
		}
	}

	for (int j = 0; j < buf->width; j += 2)
	{
		s->spaces[j] = (int) rand() % buf->height + 1;
		s->length[j] = (int) rand() % (buf->height - 3) + 3;
		s->grid[1][j].val = ' ';
		s->updates[j] = (int) rand() % 3 + 1;
	}
}

static void matrix_free(struct term_buf* buf)
{
	free(buf->astate.matrix->grid[0]);
	free(buf->astate.matrix->grid);
	free(buf->astate.matrix->length);
	free(buf->astate.matrix->spaces);
	free(buf->astate.matrix->updates);
	free(buf->astate.matrix);
}

void animate_init(struct term_buf* buf)
{
	if (config.animate)
	{
		switch(config.animation)
		{
			case 0:
			{
				doom_init(buf);
				break;
			}
			case 1:
			{
				matrix_init(buf);
				break;
			}
		}
	}
}

static void doom(struct term_buf* term_buf)
{
	static struct tb_cell fire[DOOM_STEPS] =
	{
		{' ', 9, 0}, // default
		{0x2591, 2, 0}, // red
		{0x2592, 2, 0}, // red
		{0x2593, 2, 0}, // red
		{0x2588, 2, 0}, // red
		{0x2591, 4, 2}, // yellow
		{0x2592, 4, 2}, // yellow
		{0x2593, 4, 2}, // yellow
		{0x2588, 4, 2}, // yellow
		{0x2591, 8, 4}, // white
		{0x2592, 8, 4}, // white
		{0x2593, 8, 4}, // white
		{0x2588, 8, 4}, // white
	};

	u16 src;
	u16 random;
	u16 dst;

	u16 w = term_buf->init_width;
	u8* tmp = term_buf->astate.doom->buf;

	if ((term_buf->width != term_buf->init_width) || (term_buf->height != term_buf->init_height))
	{
		return;
	}

	struct tb_cell* buf = tb_cell_buffer();

	for (u16 x = 0; x < w; ++x)
	{
		for (u16 y = 1; y < term_buf->init_height; ++y)
		{
			src = y * w + x;
			random = ((rand() % 7) & 3);
			dst = src - random + 1;

			if (w > dst)
			{
				dst = 0;
			}
			else
			{
				dst -= w;
			}

			tmp[dst] = tmp[src] - (random & 1);

			if (tmp[dst] > 12)
			{
				tmp[dst] = 0;
			}

			buf[dst] = fire[tmp[dst]];
			buf[src] = fire[tmp[src]];
		}
	}
}

// Adapted from cmatrix
static void matrix(struct term_buf* buf)
{
	static int frame = 3;
	const int frame_delay = 8;
	static int count = 0;
	bool first_col;
	struct matrix_state* s = buf->astate.matrix;

	// Allowed codepoints
	const int randmin = 33;
	const int randnum = 123 - randmin;
	// Chars change mid-scroll
	const bool changes = true;

	if ((buf->width != buf->init_width) || (buf->height != buf->init_height))
	{
		return;
	}

	count += 1;
	if (count > frame_delay) {
		frame += 1;
		if (frame > 4) frame = 1;
		count = 0;

		for (int j = 0; j < buf->width; j += 2)
		{
			int tail;
			if (frame > s->updates[j])
			{
				if (s->grid[0][j].val == -1 && s->grid[1][j].val == ' ')
				{
					if (s->spaces[j] > 0)
					{
						s->spaces[j]--;
					} else {
						s->length[j] = (int) rand() % (buf->height - 3) + 3;
						s->grid[0][j].val = (int) rand() % randnum + randmin;
						s->spaces[j] = (int) rand() % buf->height + 1;
					}
				}

				int i = 0, seg_len = 0;
				first_col = 1;
				while (i <= buf->height)
				{
					// Skip over spaces
					while (i <= buf->height
							&& (s->grid[i][j].val == ' ' || s->grid[i][j].val == -1))
					{
						i++;
					}

					if (i > buf->height) break;

					// Find the head of this col
					tail = i;
					seg_len = 0;
					while (i <= buf->height
							&& (s->grid[i][j].val != ' ' && s->grid[i][j].val != -1))
					{
						s->grid[i][j].is_head = false;
						if (changes)
						{
							if (rand() % 8 == 0)
								s->grid[i][j].val = (int) rand() % randnum + randmin;
						}
						i++;
						seg_len++;
					}

					// Head's down offscreen
					if (i > buf->height)
					{
						s->grid[tail][j].val = ' ';
						continue;
					}

					s->grid[i][j].val = (int) rand() % randnum + randmin;
					s->grid[i][j].is_head = true;

					if (seg_len > s->length[j] || !first_col) {
						s->grid[tail][j].val = ' ';
						s->grid[0][j].val = -1;
					}
					first_col = 0;
					i++;
				}
			}
		}
	}

	u32 blank;
	utf8_char_to_unicode(&blank, " ");

	for (int j = 0; j < buf->width; j += 2) {
		for (int i = 1; i <= buf->height; ++i)
		{
			u32 c;
			int fg = TB_GREEN;
			int bg = TB_DEFAULT;

			if (s->grid[i][j].val == -1 || s->grid[i][j].val == ' ')
			{
				tb_change_cell(j, i - 1, blank, fg, bg);
				continue;
			}

			char tmp[2];
			tmp[0] = s->grid[i][j].val;
			tmp[1] = '\0';
			if(utf8_char_to_unicode(&c, tmp))
			{
				if (s->grid[i][j].is_head)
				{
					fg = TB_WHITE | TB_BOLD;
				}
				tb_change_cell(j, i - 1, c, fg, bg);
			}
		}
	}
}

void animate(struct term_buf* buf)
{
	buf->width = tb_width();
	buf->height = tb_height();

	if (config.animate)
	{
		switch(config.animation)
		{
			case 0:
			{
				doom(buf);
				break;
			}
			case 1:
			{
				matrix(buf);
				break;
			}
		}
	}
}

bool cascade(struct term_buf* term_buf, u8* fails)
{
	u16 width = term_buf->width;
	u16 height = term_buf->height;

	struct tb_cell* buf = tb_cell_buffer();
	bool changes = false;
	char c_under;
	char c;

	for (int i = height - 2; i >= 0; --i)
	{
		for (int k = 0; k < width; ++k)
		{
			c = buf[i * width + k].ch;

			if (isspace(c))
			{
				continue;
			}

			c_under = buf[(i + 1) * width + k].ch;
			
			if (!isspace(c_under))
			{
				continue;
			}

			if (!changes)
			{
				changes = true;
			}

			if ((rand() % 10) > 7)
			{
				continue;
			}

			buf[(i + 1) * width + k] = buf[i * width + k];
			buf[i * width + k].ch = ' ';
		}
	}

	// stop force-updating 
	if (!changes)
	{
		sleep(7);
		*fails = 0;

		return false;
	}

	// force-update
	return true;
}
