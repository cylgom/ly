#ifndef H_LY_DRAW
#define H_LY_DRAW

#include "termbox.h"
#include "ctypes.h"

#include "inputs.h"

struct box
{
	u32 left_up;
	u32 left_down;
	u32 right_up;
	u32 right_down;
	u32 top;
	u32 bot;
	u32 left;
	u32 right;
};

struct matrix_dot
{
	int val;
	bool is_head;
};

struct matrix_state
{
	struct matrix_dot** grid;
	int* length;
	int* spaces;
	int* updates;
};

struct doom_state
{
	u8* buf;
};

union anim_state
{
	struct doom_state* doom;
	struct matrix_state* matrix;
};

struct term_buf
{
	u16 width;
	u16 height;
	u16 init_width;
	u16 init_height;

	struct box box_chars;
	char* info_line;
	u16 labels_max_len;
	u16 box_x;
	u16 box_y;
	u16 box_width;
	u16 box_height;

	union anim_state astate;
};

void draw_init(struct term_buf* buf);
void draw_free(struct term_buf* buf);
void draw_box(struct term_buf* buf);

struct tb_cell* strn_cell(char* s, u16 len);
struct tb_cell* str_cell(char* s);

void draw_labels(struct term_buf* buf);
void draw_f_commands();
void draw_lock_state(struct term_buf* buf);
void draw_desktop(struct desktop* target);
void draw_input(struct text* input);
void draw_input_mask(struct text* input);

void position_input(
	struct term_buf* buf,
	struct desktop* desktop,
	struct text* login,
	struct text* password);

void animate_init(struct term_buf* buf);
void animate(struct term_buf* buf);
bool cascade(struct term_buf* buf, u8* fails);

#endif
