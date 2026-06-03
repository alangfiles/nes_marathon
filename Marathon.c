/* Alan Files NES Marathon Runner
 * Based off code and tutorials by NES Doug
 */	

 /* 
  TODO List:
	- test game
	- add SFX and music
 */

 
 
#include "LIB/neslib.h"
#include "LIB/nesdoug.h"
#include "LIB/padlib.h"


#include "Marathon.h"
#include "SCREENS/citybegin.h"
#include "SCREENS/citybuilding.h"
#include "SCREENS/citycross.h"
#include "SCREENS/citycross2.h"
#include "SCREENS/cityend.h"
#include "SCREENS/citygeneric.h"
#include "SCREENS/trackflowers.h"
#include "SCREENS/trackbegin.h"
#include "SCREENS/trackhud.h"
#include "SCREENS/trackend.h"
#include "SCREENS/tracktrashcan.h"
#include "SCREENS/trackgeneric.h"
#include "SCREENS/waterbegin.h"
#include "SCREENS/waterbench.h"
#include "SCREENS/waterend.h"
#include "SCREENS/watergeneric.h"
#include "SCREENS/titlespecial.h"
#include "SCREENS/grandstand.h"
#include "sprites.h"

enum {
	THEME_TRACK,
	THEME_CITY,
	THEME_WATER
};

#define HUD_TILE_BYTES 448u
#define COMPACT_ROOM_FIRST_ROW (HUD_TILE_BYTES >> 5)

/*
 * Room data contract:
 * - All gameplay screen headers are compact (576 bytes).
 * - The trimmed top region (first HUD_TILE_BYTES of a full 1024-byte nametable)
 *   is stored once in trackhud[] and used as the shared donor.
 * - Compact room reads subtract HUD_TILE_BYTES; rows above that boundary use
 *   trackhud[] so sprite-zero split/HUD seams stay stable.
 */

enum {
	PHASE_BEGIN,
	PHASE_MIDDLE,
	PHASE_END
};

const unsigned char *screen_slots[2];
unsigned char screen_slots_compact[2];
unsigned char world_theme;
unsigned char world_phase;
unsigned char world_middle_remaining;
unsigned char world_pending_citycross2;

const unsigned char *next_world_screen(void){
	unsigned char roll;

	if(world_phase == PHASE_BEGIN){
		world_phase = PHASE_MIDDLE;
		world_middle_remaining = (rand8() & 3) + 2;

		if(world_theme == THEME_TRACK){
			return trackbegin;
		} else if(world_theme == THEME_CITY){
			return citybegin;
		}
		return waterbegin;
	}

	if(world_phase == PHASE_MIDDLE){
		const unsigned char *middle;

		if(world_theme == THEME_TRACK){
			roll = rand8() & 3;
			if(roll == 0){
				middle = trackflowers;
			} else if(roll == 1){
				middle = trackgeneric;
			} else {
				middle = tracktrashcan;
			}
		} else if(world_theme == THEME_CITY){
			if(world_pending_citycross2 != 0){
				world_pending_citycross2 = 0;
				middle = citycross2;
			} else {
				roll = rand8() & 3;
				if(world_middle_remaining <= 1){
					roll &= 1;
					if(roll == 0){
						middle = citybuilding;
					} else {
						middle = citygeneric; 
					}
				} else if(roll == 0){
					middle = citybuilding;
				} else if(roll == 1){
					world_pending_citycross2 = 1;
					middle = citycross;
				} else {
					middle = citygeneric;
				}
			}
		} else {
			roll = rand8() & 1;
			if(roll == 0){
				middle = waterbench;
			} else {
				middle = watergeneric;
			}
		}

		if(world_middle_remaining > 0){
			--world_middle_remaining;
		}
		if(world_middle_remaining == 0){
			world_phase = PHASE_END;
		}
		return middle;
	}

	if(world_theme == THEME_TRACK){
		world_theme = THEME_CITY;
		world_phase = PHASE_BEGIN;
		return trackend;
	}

	if(world_theme == THEME_CITY){
		world_theme = THEME_WATER;
		world_phase = PHASE_BEGIN;
		return cityend;
	}

	world_theme = THEME_TRACK;
	world_phase = PHASE_BEGIN;
	return waterend;
}

void init_world_sequence(void){
	world_theme = THEME_TRACK;
	world_phase = PHASE_BEGIN;
	world_middle_remaining = 0;
	world_pending_citycross2 = 0;
}

void queue_room_column(unsigned char room_index, unsigned char nametable, unsigned char column){
	unsigned char row;
	unsigned char source_row;
	unsigned int attr_addr;
	unsigned int data_adjust;
	const unsigned char *room_data;

	room_data = screen_slots[room_index & 1];
	if(room_data == 0){
		room_data = trackgeneric;
	}
	data_adjust = (screen_slots_compact[room_index & 1] != 0) ? HUD_TILE_BYTES : 0;

	for(row = 0; row < 22; ++row){
		source_row = row + 8;
		if((data_adjust != 0) && (source_row < COMPACT_ROOM_FIRST_ROW)){
			column_buffer[row] = trackhud[column + (((unsigned int)source_row) << 5)];
		} else {
			column_buffer[row] = room_data[column + (((unsigned int)source_row) << 5) - data_adjust];
		}
	}
	multi_vram_buffer_vert(column_buffer, 22, NTADR_A(column, 8) + ((unsigned int)nametable << 10));

	attr_addr = 0x23c0 + (column >> 2) + 16 + ((unsigned int)nametable << 10);
	for(row = 0; row < 6; ++row){
		attribute_buffer[row] = room_data[0x3c0 + (column >> 2) + (((unsigned int)(row + 2)) << 3) - data_adjust];
		one_vram_buffer(attribute_buffer[row], attr_addr + (((unsigned int)row) << 3));
	}
}

unsigned char stream_column_chunk(void){
	unsigned char row;
	unsigned char source_row;
	unsigned int nt_base;
	unsigned int attr_addr;
	unsigned int data_adjust;
	const unsigned char *room_data;

	if(stream_active == 0){
		return 0;
	}

	if(stream_room_index == 0){
		room_data = screen_slots[0];
	} else {
		room_data = screen_slots[1];
	}
	if(room_data == 0){
		room_data = trackgeneric;
	}
	data_adjust = (screen_slots_compact[stream_room_index & 1] != 0) ? HUD_TILE_BYTES : 0;

	nt_base = ((unsigned int)stream_nametable << 10);

	if(stream_stage == 0){
		for(row = 0; row < 8; ++row){
			source_row = row + 8;
			if((data_adjust != 0) && (source_row < COMPACT_ROOM_FIRST_ROW)){
				column_buffer[row] = trackhud[stream_column + (((unsigned int)source_row) << 5)];
			} else {
				column_buffer[row] = room_data[stream_column + (((unsigned int)source_row) << 5) - data_adjust];
			}
		}
		multi_vram_buffer_vert(column_buffer, 8, NTADR_A(stream_column, 8) + nt_base);
		stream_stage = 1;
		return 1;
	}

	if(stream_stage == 1){
		for(row = 0; row < 8; ++row){
			source_row = row + 16;
			if((data_adjust != 0) && (source_row < COMPACT_ROOM_FIRST_ROW)){
				column_buffer[row] = trackhud[stream_column + (((unsigned int)source_row) << 5)];
			} else {
				column_buffer[row] = room_data[stream_column + (((unsigned int)source_row) << 5) - data_adjust];
			}
		}
		multi_vram_buffer_vert(column_buffer, 8, NTADR_A(stream_column, 16) + nt_base);
		stream_stage = 2;
		return 1;
	}

	if(stream_stage == 2){
		for(row = 0; row < 6; ++row){
			source_row = row + 24;
			if((data_adjust != 0) && (source_row < COMPACT_ROOM_FIRST_ROW)){
				column_buffer[row] = trackhud[stream_column + (((unsigned int)source_row) << 5)];
			} else {
				column_buffer[row] = room_data[stream_column + (((unsigned int)source_row) << 5) - data_adjust];
			}
		}
		multi_vram_buffer_vert(column_buffer, 6, NTADR_A(stream_column, 24) + nt_base);
		stream_stage = 3;
		return 1;
	}

	attr_addr = 0x23c0 + (stream_column >> 2) + 16 + nt_base;
	for(row = 0; row < 6; ++row){
		attribute_buffer[row] = room_data[0x3c0 + (stream_column >> 2) + (((unsigned int)(row + 2)) << 3) - data_adjust];
		one_vram_buffer(attribute_buffer[row], attr_addr + (((unsigned int)row) << 3));
	}
	stream_active = 0;
	stream_stage = 0;
	return 1;
}

void draw_full_room(const unsigned char *room_data, unsigned char room_compact, unsigned char nametable){

	if(room_data == 0){
		room_data = trackgeneric;
	}

	vram_adr(NAMETABLE_A + ((unsigned int)nametable << 10));
	for(largeindex = 0; largeindex < 1024; ++largeindex){
		if((room_compact != 0) && (largeindex < HUD_TILE_BYTES)){
			vram_put(trackhud[largeindex]);
		} else if(room_compact != 0) {
			vram_put(room_data[largeindex - HUD_TILE_BYTES]);
		} else {
			vram_put(room_data[largeindex]);
		}
	}
}

const unsigned char palette_sprites[16]={
	 0x0f,0x26,0x07,0x21,
	 0x0f,0x30,0x15,0x11,
	 0x0f,0x36,0x06,0x15,
	 0x0f,0x17,0x07,0x0f };

const unsigned char palette_bg[16]={ 
	0x0f,0x00,0x1a,0x10,
	0x0f,0x11,0x1a,0x21,
	0x0f,0x2a,0x1a,0x09,
	0x0f,0x17,0x1a,0x05 };

const unsigned char palette_title[16]={
	0x0f,0x00,0x1a,0x10,
	0x0f,0x11,0x01,0x15,
	0x0f,0x2a,0x1a,0x09,
	0x0f,0x17,0x1a,0x05 };

const unsigned char palette_grandstand[16]={
	0x0f,0x21,0x15,0x10,
	0x0f,0x27,0x1c,0x36,
	0x0f,0x2a,0x1a,0x09,
	0x0f,0x17,0x1a,0x05};

const unsigned char next_digit10[10] = {1,2,3,4,5,6,7,8,9,0};
const unsigned char next_digit6[6] = {1,2,3,4,5,0};
const unsigned char next_digit3[3] = {1,2,0};


// do after the read
void process_powerpad(void){ 
	
	powerpad_new = (powerpad_cur^powerpad_old)&powerpad_cur;
	
	powerpad_old = powerpad_cur;
}	

const unsigned int powerpad_button_masks[12] = {
	POWERPAD_1, POWERPAD_2, POWERPAD_3, POWERPAD_4,
	POWERPAD_5, POWERPAD_6, POWERPAD_7, POWERPAD_8,
	POWERPAD_9, POWERPAD_10, POWERPAD_11, POWERPAD_12
};

const unsigned int powerpad_any_button_mask =
	POWERPAD_1 | POWERPAD_2 | POWERPAD_3 | POWERPAD_4 |
	POWERPAD_5 | POWERPAD_6 | POWERPAD_7 | POWERPAD_8 |
	POWERPAD_9 | POWERPAD_10 | POWERPAD_11 | POWERPAD_12;

unsigned int get_powerpad_button_mask(unsigned char button){
	if((button >= 1) && (button <= 12)){
		return powerpad_button_masks[button - 1];
	}
	return 0;
}

unsigned char random_button_value(void){
	unsigned char value;

	value = rand8() & 0x0f;
	if(value >= 12){
		value -= 12;
	}
	return value + 1;
}

const unsigned char * const big_button_sprites[12] = {
	marathon_man_1big_data,
	marathon_man_2big_data,
	marathon_man_3big_data,
	marathon_man_4big_data,
	marathon_man_5big_data,
	marathon_man_6big_data,
	marathon_man_7big_data,
	marathon_man_8big_data,
	marathon_man_9big_data,
	marathon_man_10big_data,
	marathon_man_11big_data,
	marathon_man_12big_data
};

const unsigned char *get_big_button_sprite(unsigned char button){
	if((button >= 1) && (button <= 12)){
		return big_button_sprites[button - 1];
	}
	return big_button_sprites[11];
}

void update_streak_digits(void){
	temp_int = streak;

	hundreds_streak = 0;
	while(temp_int >= 100){
		temp_int -= 100;
		++hundreds_streak;
	}

	tens_streak = 0;
	while(temp_int >= 10){
		temp_int -= 10;
		++tens_streak;
	}

	ones_streak = (unsigned char)temp_int;
}

void reset_streak(void){
	streak = 0;
	update_streak_digits();
}

void add_streak_hit(void){
	if(streak < 999u){
		++streak;
	}
	update_streak_digits();
}

const unsigned int target_speed_table[11] = {
	205, 255, 305, 355, 405, 456, 506, 556, 606, 656, 706
};

unsigned int get_target_speed_fp(void){
	unsigned char idx = (unsigned char)streak;
	if(idx > TARGET_MAX_SPEED_STREAK){
		idx = TARGET_MAX_SPEED_STREAK;
	}
	return target_speed_table[idx];
}

void spawn_target_button(void){
	target_button = random_button_value();
	target_mask = get_powerpad_button_mask(target_button);
	target_sprite_data = get_big_button_sprite(target_button);
	target_x = TARGET_START_X;
	target_x_fp = ((unsigned int)TARGET_START_X) << 8;
	target_speed_fp = get_target_speed_fp();
	target_spawn_timer = 0;
}

void init_target_button(void){
	spawn_target_button();
}

void update_target_button(void){
	if(target_x_fp > target_speed_fp){
		target_x_fp -= target_speed_fp;
	} else {
		target_x_fp = 0;
	}
	target_x = (unsigned char)(target_x_fp >> 8);

	++target_spawn_timer;
	if(target_spawn_timer >= TARGET_RESPAWN_FRAMES || target_x == 0){
		if(streak > 0){
			sfx_play(SFX_STREAK_END, 0);
		}
		reset_streak();
		spawn_target_button();
	}
}

void add_score(void){
	
	score += score_to_add;

	while(score_to_add > 0){
		--score_to_add;

		ones_score = next_digit10[ones_score];
		if(ones_score != 0){
			continue;
		}

		tens_score = next_digit10[tens_score];
		if(tens_score != 0){
			continue;
		}

		hundreds_score = next_digit10[hundreds_score];
		if(hundreds_score != 0){
			continue;
		}

		thousands_score = next_digit10[thousands_score];
		if(thousands_score == 0 && ten_thousands_score < 9){
			++ten_thousands_score;
		}
	}
}

void draw_target_button(void){
	if(target_x > 8){
		oam_meta_spr(target_x, 83, target_sprite_data);
	}
}

void init_hud_labels(void){
	multi_vram_buffer_horz("STEPS:", 6, NTADR_A(2, 4));
	multi_vram_buffer_horz("TIME:", 5, NTADR_A(2, 2));
	multi_vram_buffer_horz("SCORE:", 6, NTADR_A(18, 2));
	multi_vram_buffer_horz("STREAK:", 7, NTADR_A(18, 4));
}


void build_options_nametable(unsigned char nametable){
	unsigned char row;
	unsigned int nt_base;

	nt_base = ((unsigned int)nametable << 10);
	draw_full_room(trackbegin, 1, nametable);

	// Keep the track-begin lower section, but clear the upper section for menu text.
	for(row = 0; row < 14; ++row){
		vram_adr(NAMETABLE_A + nt_base + (((unsigned int)row) << 5));
		vram_fill(0x00, 32);
	}

	multi_vram_buffer_horz("SELECT RACE", 11, NTADR_A(7, 3) + nt_base);

	multi_vram_buffer_horz("1K", 2, NTADR_A(8, 6) + nt_base);

	multi_vram_buffer_horz("5K", 2, NTADR_A(8, 8) + nt_base);

	multi_vram_buffer_horz("10K", 3, NTADR_A(8, 10) + nt_base);

	multi_vram_buffer_horz("MARATHON", 8, NTADR_A(8, 12) + nt_base);
}

void clear_options_top_rows(unsigned char nametable){
	unsigned char row;
	unsigned int nt_base;

	nt_base = ((unsigned int)nametable << 10);
	for(row = 0; row < 14; ++row){
		vram_adr(NAMETABLE_A + nt_base + (((unsigned int)row) << 5));
		vram_fill(0x00, 32);
	}
}

void reset_secret_start_values(void){
	secret_steps_digits[0] = 0;
	secret_steps_digits[1] = 0;
	secret_steps_digits[2] = 0;
	secret_steps_digits[3] = 0;
	secret_steps_digits[4] = 0;
	secret_time_hours = 0;
	secret_time_minutes_tens = 0;
	secret_time_minutes_ones = 0;
	secret_cursor_index = 0;
}

void apply_secret_start_values(void){
	unsigned char i;
	unsigned char digit;
	unsigned int step_value;
	unsigned int minute_value;

	step_value = 0;
	for(i = 0; i < 5; ++i){
		digit = secret_steps_digits[i];
		if((step_value > 6553u) || ((step_value == 6553u) && (digit > 5u))){
			step_value = 65535u;
		} else {
			step_value = (unsigned int)(step_value * 10u + digit);
		}
	}

	minute_value = (unsigned int)(secret_time_minutes_tens * 10u + secret_time_minutes_ones);
	configured_start_steps = step_value;
	configured_start_seconds = (unsigned int)(secret_time_hours * 3600u + minute_value * 60u);
	secret_start_override_active = 1;
}

void init_secret_screen(void){
	ppu_off();
	pal_bg(palette_bg);
	pal_spr(palette_sprites);
	oam_clear();
	clear_vram_buffer();

	vram_adr(NAMETABLE_A);
	vram_fill(0x00, 1024);

	reset_secret_start_values();

	multi_vram_buffer_horz("MARATHON RESTART", 16, NTADR_A(7, 9));
	multi_vram_buffer_horz("STEPS: 00000", 12, NTADR_A(8, 11));
	multi_vram_buffer_horz("TIME 0H, 00M", 12, NTADR_A(8, 13));

	game_mode = MODE_SECRET;
	set_scroll_x(0);
	set_scroll_y(0);
	ppu_on_all();
}

void draw_secret_screen(void){
	unsigned char cursor_col;
	unsigned char cursor_row;

	oam_clear();

	debug_controller = pad_poll(0);
	debug_controller_new = get_pad_new(0);

	if(debug_controller_new & PAD_LEFT){
		if(secret_cursor_index > 0){
			--secret_cursor_index;
		}
	}

	if(debug_controller_new & PAD_RIGHT){
		if(secret_cursor_index < 7){
			++secret_cursor_index;
		}
	}

	if((debug_controller_new & PAD_UP) || (debug_controller_new & PAD_A)){
		if(secret_cursor_index < 5){
			++secret_steps_digits[secret_cursor_index];
			if(secret_steps_digits[secret_cursor_index] > 9){
				secret_steps_digits[secret_cursor_index] = 0;
			}
		} else if(secret_cursor_index == 5){
			++secret_time_hours;
			if(secret_time_hours > 9){
				secret_time_hours = 0;
			}
		} else if(secret_cursor_index == 6){
			++secret_time_minutes_tens;
			if(secret_time_minutes_tens > 5){
				secret_time_minutes_tens = 0;
			}
		} else {
			++secret_time_minutes_ones;
			if(secret_time_minutes_ones > 9){
				secret_time_minutes_ones = 0;
			}
		}
	}

	if((debug_controller_new & PAD_DOWN) || (debug_controller_new & PAD_B)){
		if(secret_cursor_index < 5){
			if(secret_steps_digits[secret_cursor_index] == 0){
				secret_steps_digits[secret_cursor_index] = 9;
			} else {
				--secret_steps_digits[secret_cursor_index];
			}
		} else if(secret_cursor_index == 5){
			if(secret_time_hours == 0){
				secret_time_hours = 9;
			} else {
				--secret_time_hours;
			}
		} else if(secret_cursor_index == 6){
			if(secret_time_minutes_tens == 0){
				secret_time_minutes_tens = 5;
			} else {
				--secret_time_minutes_tens;
			}
		} else {
			if(secret_time_minutes_ones == 0){
				secret_time_minutes_ones = 9;
			} else {
				--secret_time_minutes_ones;
			}
		}
	}

	if(debug_controller_new & PAD_SELECT){
		reset_secret_start_values();
	}

	one_vram_buffer(0x30 + secret_steps_digits[0], NTADR_A(15, 11));
	one_vram_buffer(0x30 + secret_steps_digits[1], NTADR_A(16, 11));
	one_vram_buffer(0x30 + secret_steps_digits[2], NTADR_A(17, 11));
	one_vram_buffer(0x30 + secret_steps_digits[3], NTADR_A(18, 11));
	one_vram_buffer(0x30 + secret_steps_digits[4], NTADR_A(19, 11));
	one_vram_buffer(0x30 + secret_time_hours, NTADR_A(13, 13));
	one_vram_buffer(0x30 + secret_time_minutes_tens, NTADR_A(17, 13));
	one_vram_buffer(0x30 + secret_time_minutes_ones, NTADR_A(18, 13));

	one_vram_buffer(0x00, NTADR_A(15, 12));
	one_vram_buffer(0x00, NTADR_A(16, 12));
	one_vram_buffer(0x00, NTADR_A(17, 12));
	one_vram_buffer(0x00, NTADR_A(18, 12));
	one_vram_buffer(0x00, NTADR_A(19, 12));
	one_vram_buffer(0x00, NTADR_A(13, 14));
	one_vram_buffer(0x00, NTADR_A(17, 14));
	one_vram_buffer(0x00, NTADR_A(18, 14));

	if(secret_cursor_index < 5){
		cursor_col = (unsigned char)(15 + secret_cursor_index);
		cursor_row = 12;
	} else if(secret_cursor_index == 5){
		cursor_col = 13;
		cursor_row = 14;
	} else if(secret_cursor_index == 6){
		cursor_col = 17;
		cursor_row = 14;
	} else {
		cursor_col = 18;
		cursor_row = 14;
	}

	one_vram_buffer(0x3f, NTADR_A(cursor_col, cursor_row));

	if(debug_controller_new & PAD_START){
		apply_secret_start_values();
		selected_option = RACE_MARATHON;
		init_mode_game();
	}
}

void begin_title_to_options(void){
	pal_bg(palette_bg);
	pal_spr(palette_sprites);
	selected_option = RACE_1K;
	options_cursor_frame = 0;
	options_cursor_timer = 0;
	options_phase = OPTIONS_PHASE_IDLE;
	options_runner_x = 24;
	options_runner_timer = 0;
	options_runner_frame = 0;
	options_runner_hold_timer = 0;
	title_scroll_x = 0;
	title_runner_x = 24;
	title_transition_phase = 0;
	title_animation_frame = 0;
	title_frame_counter = 0;
	game_mode = MODE_TITLE_TO_OPTIONS;
	set_scroll_x(0);
	set_scroll_y(0);
}

const unsigned char *get_run_frame_0_to_5(unsigned char frame_index){
	if(frame_index == 0){
		return marathon_man_run1_data;
	} else if(frame_index == 1){
		return marathon_man_run2_data;
	} else if(frame_index == 2){
		return marathon_man_run3_data;
	} else if(frame_index == 3){
		return marathon_man_run4_data;
	} else if(frame_index == 4){
		return marathon_man_run5_data;
	}
	return marathon_man_run6_data;
}

const unsigned char *get_run_frame_0_to_59(unsigned char frame_counter){
	if(frame_counter < 10){
		return marathon_man_run1_data;
	} else if(frame_counter < 20){
		return marathon_man_run2_data;
	} else if(frame_counter < 30){
		return marathon_man_run3_data;
	} else if(frame_counter < 40){
		return marathon_man_run4_data;
	} else if(frame_counter < 50){
		return marathon_man_run5_data;
	}
	return marathon_man_run6_data;
}

void draw_title_to_options(void){
	const unsigned char *runner;

	oam_clear();

	++title_frame_counter;
	if(title_frame_counter >= 6){
		title_frame_counter = 0;
		++title_animation_frame;
		if(title_animation_frame >= 6){
			title_animation_frame = 0;
		}
	}

	runner = get_run_frame_0_to_5(title_animation_frame);

	// After START: keep runner on the left and begin scrolling immediately.
	oam_meta_spr(24, 120, runner);
	if(title_scroll_x <= 253){
		title_scroll_x += 2;
	} else {
		title_scroll_x = 255;
	}

	set_scroll_x(title_scroll_x);
	set_scroll_y(0);

	if(title_scroll_x >= 255){
		game_mode = MODE_OPTIONS;
	}
}

const unsigned char *get_end_screen_for_room(const unsigned char *room_data){
	if((room_data == trackbegin) || (room_data == trackflowers) || (room_data == tracktrashcan) ||
	   (room_data == trackgeneric) || (room_data == trackend)){
		return trackend;
	}

	if((room_data == citybegin) || (room_data == citybuilding) || (room_data == citycross) ||
	   (room_data == citycross2) || (room_data == citygeneric) || (room_data == cityend)){
		return cityend;
	}

	if((room_data == waterbegin) || (room_data == waterbench) ||
	   (room_data == watergeneric) || (room_data == waterend)){
		return waterend;
	}

	return trackend;
}

void activate_grandstand_section(void){
	unsigned char current_room_index;
	const unsigned char *current_room;

	ending_sequence_active = 1;
	ending_queue_stage = 0;
	ending_grandstand_active = 0;
	current_room_index = (unsigned char)((scroll_x >> 8) & 1u);
	current_room = screen_slots[current_room_index];
	ending_forced_end_screen = get_end_screen_for_room(current_room);
}

void update_endgame_state(void){
	unsigned int remaining_steps;
	unsigned int ending_sequence_trigger_steps;

	if(steps >= total_steps_needed){
		remaining_steps = 0;
	} else {
		remaining_steps = total_steps_needed - steps;
	}

	ending_sequence_trigger_steps = ENDING_SEQUENCE_STEPS;
	if(race_type == RACE_1K){
		ending_sequence_trigger_steps = 100u;
	}

	if((ending_sequence_active == 0) && (remaining_steps <= ending_sequence_trigger_steps)){
		activate_grandstand_section();
	}

	if((ending_runoff_active == 0) && (remaining_steps <= FINAL_RUNOFF_STEPS)){
		ending_runoff_active = 1;
		runner_screen_x = 120;
		velocity = 0;
		scroll_subpixel = 0;
		stream_active = 0;
		stream_stage = 0;
		motion = RUNNING;
	}
}


void init_mode_game(void){
	unsigned int steps_per_pixel;

	ppu_off();
	pal_bg(palette_bg);
	pal_spr(palette_sprites);
	music_stop();
	sfx_play(SFX_START, 0);


	//set race type
	race_type = selected_option;
	if(race_type == RACE_1K){
		total_steps_needed = 1000u;
	} else if(race_type == RACE_5K){
		total_steps_needed = 5000u; 
	} else if (race_type == RACE_10K){
		total_steps_needed = 10000u;
	} else {
		total_steps_needed = 42195u;
	}

	clear_vram_buffer();

	// Optional debug override from secret title menu.
	if(secret_start_override_active != 0){
		steps = configured_start_steps;
		seconds = configured_start_seconds;
		secret_start_override_active = 0;
	} else {
		steps = 0;
		seconds = 0;
	}

	// mess with these for debug, but they should all be 0
	score = 0;

	scroll_x = 0;
	scroll_subpixel = 0;
	last_stream_column = 0xffff;
	stream_active = 0;
	stream_stage = 0;
	velocity = 0; 
	frame_counter = 0;
	scroll_timer = 0;
	step_button_lockout = 0;
	if(steps >= total_steps_needed){
		progress_pixels = 212;
		progress_remainder = 0;
	} else {
		steps_per_pixel = total_steps_needed / 212u;
		if(steps_per_pixel == 0){
			steps_per_pixel = 1;
		}
		progress_pixels = (unsigned char)(steps / steps_per_pixel);
		if(progress_pixels > 212){
			progress_pixels = 212;
		}
		progress_remainder = 0;
	}
	ending_sequence_active = 0;
	ending_queue_stage = 0;
	ending_forced_end_screen = trackend;
	ending_grandstand_active = 0;
	ending_runoff_active = 0;
	runner_screen_x = 120;
	sprite_frame_counter = 0;
	time_since_button_press = 255;
	motion = STANDING;
	powerpad_old = 0;
	powerpad_new = 0;
	last_step = 0;
	ones_score = 0;
	tens_score = 0;
	hundreds_score = 0;
	thousands_score = 0;
	ten_thousands_score = 0;
	reset_streak();
	init_target_button();

	initial_steps_conversion();
	initial_timer_conversion();

	load_room_pair();
	ppu_off();
	init_hud_labels();
	set_sprite_zero();
	game_intro_timer = 0;
	set_scroll_x(0);
	set_scroll_y(0);

	game_mode = MODE_GAME_INTRO;
	ppu_on_all();
}

	
	
void main (void) {

	clear_vram_buffer(); 
	set_vram_buffer();

	ppu_off(); // screen off

	// use the second set of tiles for sprites
	bank_spr(1);

	game_mode = MODE_TITLE;
	load_title();
	// game_mode = MODE_GAME;
	// init_mode_game();

	ppu_on_all(); // turn on screen


	while (1){

		while(game_mode == MODE_TITLE){
			ppu_wait_nmi();

			++title_frame_counter;
			if(title_frame_counter >= 4){
				title_frame_counter = 0;
				if(title_animation_frame <= 54){
					++title_animation_frame;
				}
			}

			oam_clear();
			set_signature_sprite();
			oam_meta_spr(24, 120, marathon_man_walk1title_data);

			debug_controller = pad_poll(0);
			debug_controller_new = get_pad_new(0);

			if(debug_controller_new & PAD_START){
				if((debug_controller & (PAD_SELECT | PAD_A | PAD_B)) == (PAD_SELECT | PAD_A | PAD_B)){
					init_secret_screen();
				} else if(title_animation_frame >= 54){
					begin_title_to_options();
				}
			}
		}

		while(game_mode == MODE_TITLE_TO_OPTIONS){
			ppu_wait_nmi();
			draw_title_to_options();
		}

		while(game_mode == MODE_OPTIONS){
			ppu_wait_nmi();
			draw_options_screen();

			debug_controller = pad_poll(0);
			debug_controller_new = get_pad_new(0);

			if((options_phase == OPTIONS_PHASE_IDLE) && (debug_controller_new & PAD_UP)){
				if(selected_option > 0) --selected_option;
			}
			if((options_phase == OPTIONS_PHASE_IDLE) && (debug_controller_new & PAD_DOWN)){
				if(selected_option < 3) ++selected_option;
			}
			if((options_phase == OPTIONS_PHASE_IDLE) && (debug_controller_new & (PAD_START | PAD_A))){
				race_type = selected_option;
				ppu_off();
				clear_options_top_rows(1);
				ppu_on_all();
				oam_clear();
				options_phase = OPTIONS_PHASE_CONFIRM;
				options_runner_timer = 0;
				options_runner_frame = 0;
				options_runner_hold_timer = 0;
			}
		}

		while(game_mode == MODE_SECRET){
			ppu_wait_nmi();
			draw_secret_screen();
		}

		while(game_mode == MODE_GAME_INTRO){

			ppu_wait_nmi();

			set_scroll_x(0);
			set_scroll_y(0);

			draw_sprite();
			draw_hud();

			++game_intro_timer;
			if(game_intro_timer >= 40){
				game_mode = MODE_GAME;
			}
		}

		while(game_mode == MODE_GAME){

		ppu_wait_nmi(); // wait till beginning of the frame

		set_scroll_x(0);
		set_scroll_y(0);
		did_stream_column = 0;

		//timer stuff
		++frame_counter;
		if(time_since_button_press < 255){
			++time_since_button_press;
		}

		if(velocity > 4){
			velocity -= 4;
		} else {
			velocity = 0;
		}

		if(ending_runoff_active == 0){
			// Accumulate subpixel camera movement so low velocity still scrolls.
			scroll_subpixel += velocity;
			scroll_x += (scroll_subpixel >> 8);
			scroll_subpixel &= 0x00ff;
			if((velocity > 0) || (stream_active != 0)){
				draw_screen_R();
			} else {
				did_stream_column = 0;
			}
		} else {
			velocity = 0;
			scroll_subpixel = 0;
			stream_active = 0;
			did_stream_column = 0;
			if(runner_screen_x <= 250){
				runner_screen_x += 2;
			}
		}
		

		if(step_button_lockout > 0){
			--step_button_lockout;
		}

		if(frame_counter >= 60){
			frame_counter = 0;
			add_second();
		}
		//end timer stuff
		
		debug_controller = pad_poll(0); //for debugging only
		debug_controller_new = get_pad_new(0);  

		powerpad_cur = read_powerpad(1);
		process_powerpad(); // goes after the read
							// transfers only new presses to powerpad_new
							// powerpad_new isn't used here, but
							// would be very useful for a game

		update_target_button();

		check_motion();

		split(scroll_x);

		draw_sprite();
		if((did_stream_column == 0) || ((velocity >= 512) ? ((sprite_frame_counter & 3) == 0) : ((sprite_frame_counter & 1) == 0))){
			draw_hud();
		}

		process_controller();
			update_endgame_state();

			if((ending_runoff_active != 0) && (runner_screen_x > 250)){
			init_win_screen();
			break;
		}

		// gray_line(); // debug only, too expensive for regular gameplay
                } // end MODE_GAME

		while(game_mode == MODE_WIN){
			const unsigned char *win_runner_data;

			ppu_wait_nmi();
			oam_clear();

			++sprite_frame_counter;
			if(sprite_frame_counter >= 60){
				sprite_frame_counter = 0;
			}

			win_runner_data = get_run_frame_0_to_59(sprite_frame_counter);

			
			if(runner_screen_x <= 248u){
				oam_meta_spr(runner_screen_x, 120, win_runner_data);
				runner_screen_x += 2;
				
			}
			oam_meta_spr(184, 170, marathon_man_gate_data);
			

			debug_controller = pad_poll(0);
			debug_controller_new = get_pad_new(0);

			if((debug_controller_new & PAD_START) || (debug_controller_new & PAD_A)){
				load_title();
			}
		}

        } // end while(1)
}

void process_controller(void){
	unsigned char target_hit;

	target_hit = 0;
	if(target_mask != 0 && (powerpad_new & target_mask) && target_x <= TARGET_HIT_X){
		sfx_play(SFX_TARGET_HIT, 0);
		score_to_add = (unsigned char)(1u + streak);
		add_score();
		add_streak_hit();
		spawn_target_button();
		target_hit = 1;
	}

	if((debug_controller_new & (PAD_A | PAD_B)) || (powerpad_new & powerpad_any_button_mask)){
		play_step_sfx = target_hit ==0;
		add_step();
	}
		
}

void add_second(void){
	seconds++;

	ones_seconds = next_digit10[ones_seconds];
	if(ones_seconds != 0){
		return;
	}

	tens_seconds = next_digit6[tens_seconds];
	if(tens_seconds != 0){
		return;
	}

	ones_minutes = next_digit10[ones_minutes];
	if(ones_minutes != 0){
		return;
	}

	tens_minutes = next_digit6[tens_minutes];
	if(tens_minutes != 0){
		return;
	}

	ones_hours = next_digit10[ones_hours];
	if(ones_hours != 0){
		return;
	}

	tens_hours = next_digit3[tens_hours];
}

void add_step(){
	
	if(step_button_lockout > 0){
		return; //still in lockout period
	}
	if(last_step == 0){
		if(play_step_sfx != 0){
			sfx_play(SFX_STEP, 0);
		}
		last_step = 1;
	} else {
		if(play_step_sfx != 0){
			sfx_play(SFX_STEP2, 0);
		}
		last_step = 0;
	}
	update_steps_per_minute(); // calculate SPM before resetting the timer
	sprite_timer = 0; //used for animation
	velocity += 100;
	if(velocity > 720){
		velocity = 720;
	}

	time_since_button_press = 0;
	steps++;

	if(progress_pixels < 212){
		progress_remainder += 212;
		if(progress_remainder >= total_steps_needed){
			progress_remainder -= total_steps_needed;
			++progress_pixels;
		}
	}

	step_button_lockout = FRAMES_PER_STEP; //lock out for a few frames to avoid double counting

	if(ones_step == 9){
		ones_step = 0;
		if(tens_step == 9){
			tens_step = 0;
			if(hundreds_step == 9){
				hundreds_step = 0;
				if(thousands_step == 9){
					thousands_step = 0;
					if(ten_thousands_step == 9){
						ten_thousands_step = 0;
					} else {
						ten_thousands_step++;
					}
				} else {
					thousands_step++;
				}
			} else {
				hundreds_step++;
			}
		} else {
			tens_step++;
		}
	} else {
		ones_step++;
	}

}

void check_motion(void){
	// determine animation state directly from current velocity
	if(velocity >= 320){
		motion = RUNNING;
	} else if(velocity > 0){
		motion = WALKING;
	} else {
		motion = STANDING;
	}
}

void update_steps_per_minute(void){
	if(time_since_button_press > 0 && time_since_button_press < 255) {
		steps_per_minute = 3600u / (unsigned int)time_since_button_press;
	} else {
		steps_per_minute = 0;
	}

	temp_int = steps_per_minute;
	while(temp_int >= 1000){
		temp_int -= 1000;
	}

	hundreds_spm = 0;
	while(temp_int >= 100){
		temp_int -= 100;
		++hundreds_spm;
	}

	tens_spm = 0;
	while(temp_int >= 10){
		temp_int -= 10;
		++tens_spm;
	}

	ones_spm = (unsigned char)temp_int;
}

void initial_steps_conversion(void){
	
	temp_int = steps;

	ten_thousands_step = 0;
	while(temp_int >= 10000){
		temp_int -= 10000;
		++ten_thousands_step;
	}

	thousands_step = 0;
	while(temp_int >= 1000){
		temp_int -= 1000;
		++thousands_step;
	}

	hundreds_step = 0;
	while(temp_int >= 100){
		temp_int -= 100;
		++hundreds_step;
	}

	tens_step = 0;
	while(temp_int >= 10){
		temp_int -= 10;
		++tens_step;
	}

	ones_step = (unsigned char)temp_int;

	one_vram_buffer(0x30+ten_thousands_step, NTADR_A(10, 4));
	one_vram_buffer(0x30+thousands_step, NTADR_A(11, 4));
	one_vram_buffer(0x30+hundreds_step, NTADR_A(13, 4));
	one_vram_buffer(0x30+tens_step, NTADR_A(14, 4));
	one_vram_buffer(0x30+ones_step, NTADR_A(15, 4));
}

void initial_timer_conversion(void){
	
	temp_int = seconds;

	tens_hours = 0;
	ones_hours = 0;
	tens_minutes = 0;
	ones_minutes = 0;
	tens_seconds = 0;
	ones_seconds = 0;

	while(temp_int >= 3600){
		temp_int -= 3600;
		ones_hours = next_digit10[ones_hours];
		if(ones_hours == 0){
			tens_hours = next_digit3[tens_hours];
		}
	}

	while(temp_int >= 60){
		temp_int -= 60;
		ones_minutes = next_digit10[ones_minutes];
		if(ones_minutes == 0){
			tens_minutes = next_digit6[tens_minutes];
		}
	}

	while(temp_int >= 10){
		temp_int -= 10;
		tens_seconds = next_digit6[tens_seconds];
	}

	ones_seconds = (unsigned char)temp_int;

	one_vram_buffer(0x30+tens_hours, NTADR_A(8, 2));
	one_vram_buffer(0x30+ones_hours, NTADR_A(9, 2));
	one_vram_buffer(0x30+tens_minutes, NTADR_A(11, 2));
	one_vram_buffer(0x30+ones_minutes, NTADR_A(12, 2));
	one_vram_buffer(0x30+tens_seconds, NTADR_A(14, 2));
	one_vram_buffer(0x30+ones_seconds, NTADR_A(15, 2));

}

void draw_sprite(){
	unsigned char progress_x;
	unsigned char runner_x;
	unsigned char effective_motion;
	const unsigned char *progress_cursor_data;

	oam_clear();
	set_sprite_zero();
	oam_set(4); // start drawing from slot 4

	++sprite_frame_counter;
	if(sprite_frame_counter >= 60){
		sprite_frame_counter = 0;
	}

	if(velocity == 0){
		progress_cursor_data = marathon_man_cursor2_data;
	} else if(sprite_frame_counter < 15){
		progress_cursor_data = marathon_man_cursor1_data;
	} else if(sprite_frame_counter < 30){
		progress_cursor_data = marathon_man_cursor2_data;
	} else if(sprite_frame_counter < 45){
		progress_cursor_data = marathon_man_cursor3_data;
	} else {
		progress_cursor_data = marathon_man_cursor2_data;
	}

	progress_x = 16 + progress_pixels;

	// Draw progress cursor on the top race bar.
	oam_meta_spr(progress_x + 5, 46, progress_cursor_data);
	draw_target_button();

	runner_x = runner_screen_x;
	effective_motion = motion;
	if(ending_runoff_active != 0){
		effective_motion = RUNNING;
	}

	if(effective_motion == RUNNING){
		// 6 frames, 10 ticks each = 60-frame cycle
		oam_meta_spr(runner_x, 120, get_run_frame_0_to_59(sprite_frame_counter));
	} else if(effective_motion == WALKING){
		// 4 frames, 15 ticks each = 60-frame cycle
		if(sprite_frame_counter < 15){
			oam_meta_spr(runner_x, 120, marathon_man_walk1_data);
		} else if(sprite_frame_counter < 30){
			oam_meta_spr(runner_x, 120, marathon_man_walk2_data);
		} else if(sprite_frame_counter < 45){
			oam_meta_spr(runner_x, 120, marathon_man_walk3_data);
		} else {
			oam_meta_spr(runner_x, 120, marathon_man_walk4_data);
		}
	} else {
		// STANDING - hold a neutral walking pose
		oam_meta_spr(runner_x, 120, marathon_man_walk1_data);
	}
}


void draw_hud(void){
	one_vram_buffer(0x30+ten_thousands_step, NTADR_A(10, 4));
	one_vram_buffer(0x30+thousands_step, NTADR_A(11, 4));
	one_vram_buffer(',', NTADR_A(12, 4));
	one_vram_buffer(0x30+hundreds_step, NTADR_A(13, 4));
	one_vram_buffer(0x30+tens_step, NTADR_A(14, 4));
	one_vram_buffer(0x30+ones_step, NTADR_A(15, 4));

	one_vram_buffer(0x30+tens_hours, NTADR_A(8, 2));
	one_vram_buffer(0x30+ones_hours, NTADR_A(9, 2));
	one_vram_buffer(':', NTADR_A(10, 2));
	one_vram_buffer(0x30+tens_minutes, NTADR_A(11, 2));
	one_vram_buffer(0x30+ones_minutes, NTADR_A(12, 2));
	one_vram_buffer(':', NTADR_A(13, 2));
	one_vram_buffer(0x30+tens_seconds, NTADR_A(14, 2));
	one_vram_buffer(0x30+ones_seconds, NTADR_A(15, 2));

	one_vram_buffer(0x30+ten_thousands_score, NTADR_A(24, 2));
	one_vram_buffer(0x30+thousands_score, NTADR_A(25, 2));
	one_vram_buffer(0x30+hundreds_score, NTADR_A(26, 2));
	one_vram_buffer(0x30+tens_score, NTADR_A(27, 2));
	one_vram_buffer(0x30+ones_score, NTADR_A(28, 2));

	one_vram_buffer(0x30+hundreds_streak, NTADR_A(26, 4));
	one_vram_buffer(0x30+tens_streak, NTADR_A(27, 4));
	one_vram_buffer(0x30+ones_streak, NTADR_A(28, 4));
} 


void draw_options_screen(void){
	unsigned char cursor_y;
	const unsigned char *runner_data;
	const unsigned char *cursor_data;

	++options_cursor_timer;
	if(options_cursor_timer >= 16){
		options_cursor_timer = 0;
		++options_cursor_frame;
		if(options_cursor_frame >= 4){
			options_cursor_frame = 0;
		}
	}

	if(selected_option == 0){
		cursor_y = 48;
	} else if(selected_option == 1){
		cursor_y = 64;
	} else if(selected_option == 2){
		cursor_y = 80;
	} else {
		cursor_y = 96;
	}

	if(options_cursor_frame == 0){
		cursor_data = marathon_man_cursor1_data;
	} else if(options_cursor_frame == 1){
		cursor_data = marathon_man_cursor2_data;
	} else if(options_cursor_frame == 2){ 
		cursor_data = marathon_man_cursor3_data;
	} else {
		cursor_data = marathon_man_cursor2_data;
	}

	oam_clear();
	oam_set(0);

	if(options_phase == OPTIONS_PHASE_IDLE){
		runner_data = marathon_man_walk1_data;
		oam_meta_spr(56, cursor_y - 2, cursor_data);
	} else {
		if(options_runner_x < 120){
			++options_runner_timer;
			if(options_runner_timer >= 6){
				options_runner_timer = 0;
				++options_runner_frame;
				if(options_runner_frame >= 6){
					options_runner_frame = 0;
				}
				options_runner_x += 3;
			}

			runner_data = get_run_frame_0_to_5(options_runner_frame);
		} else {
			runner_data = marathon_man_walk1_data;
			++options_runner_hold_timer;
			if(options_runner_hold_timer >= 8){
				init_mode_game();
				return;
			}
		}
	}

	oam_meta_spr(options_runner_x, 120, runner_data);
}

void init_options(void){
	ppu_off();
	pal_bg(palette_bg);
	pal_spr(palette_sprites);
	oam_clear();
	clear_vram_buffer();

	selected_option = RACE_5K;
	options_cursor_frame = 0;
	options_cursor_timer = 0;
	options_phase = OPTIONS_PHASE_IDLE;
	options_runner_x = 24;
	options_runner_timer = 0;
	options_runner_frame = 0;
	options_runner_hold_timer = 0;

	build_options_nametable(0);

	game_mode = MODE_OPTIONS;
	set_scroll_x(0);
	set_scroll_y(0);
	ppu_on_all();
}

void init_win_screen(void){
	char time_line[14];
	char score_line[12];
	const char *race_line;
	unsigned char race_line_len;

	ppu_off();
	pal_bg(palette_grandstand);
	pal_spr(palette_sprites);
	sfx_play(SFX_START, 0);
	oam_clear();
	clear_vram_buffer();

	draw_full_room(grandstand, 1, 0);
	clear_options_top_rows(0);

	multi_vram_buffer_horz("CONGRATULATIONS", 15, NTADR_A(8, 3));
	multi_vram_buffer_horz("YOU FINISHED", 12, NTADR_A(10, 5));

	if(race_type == RACE_1K){
		race_line = "THE 1K RACE";
		race_line_len = 11;
	} else if(race_type == RACE_5K){
		race_line = "THE 5K RACE";
		race_line_len = 11;
	} else if(race_type == RACE_10K){
		race_line = "THE 10K RACE";
		race_line_len = 12;
	} else {
		race_line = "THE MARATHON";
		race_line_len = 12;
	}
	multi_vram_buffer_horz(race_line, race_line_len, NTADR_A(9, 7));

	time_line[0] = 'T';
	time_line[1] = 'I';
	time_line[2] = 'M';
	time_line[3] = 'E';
	time_line[4] = ':';
	time_line[5] = ' ';
	time_line[6] = (char)(0x30 + tens_hours);
	time_line[7] = (char)(0x30 + ones_hours);
	time_line[8] = ':';
	time_line[9] = (char)(0x30 + tens_minutes);
	time_line[10] = (char)(0x30 + ones_minutes);
	time_line[11] = ':';
	time_line[12] = (char)(0x30 + tens_seconds);
	time_line[13] = (char)(0x30 + ones_seconds);
	multi_vram_buffer_horz(time_line, 14, NTADR_A(8, 9));

	score_line[0] = 'S';
	score_line[1] = 'C';
	score_line[2] = 'O';
	score_line[3] = 'R';
	score_line[4] = 'E';
	score_line[5] = ':';
	score_line[6] = ' ';
	score_line[7] = (char)(0x30 + ten_thousands_score);
	score_line[8] = (char)(0x30 + thousands_score);
	score_line[9] = (char)(0x30 + hundreds_score);
	score_line[10] = (char)(0x30 + tens_score);
	score_line[11] = (char)(0x30 + ones_score);
	multi_vram_buffer_horz(score_line, 12, NTADR_A(9, 11));

	runner_screen_x = 0;
	sprite_frame_counter = 0;

	game_mode = MODE_WIN;
	set_scroll_x(0);
	set_scroll_y(0);
	ppu_on_all();
}

void set_signature_sprite(){
	switch(title_animation_frame){		
		case 0: pointer = marathon_man_alan0_data; break;
		case 1: pointer = marathon_man_alan1_data; break;
		case 2: pointer = marathon_man_alan2_data; break;
		case 3: pointer = marathon_man_alan3_data; break;
		case 4: pointer = marathon_man_alan4_data; break;
		case 5: pointer = marathon_man_alan5_data; break;	
		case 6: pointer = marathon_man_alan6_data; break;
		case 7: pointer = marathon_man_alan7_data; break;
		case 8: pointer = marathon_man_alan8_data; break;
		case 9: pointer = marathon_man_alan9_data; break;
		case 10: pointer = marathon_man_alan10_data; break;
		case 11: pointer = marathon_man_alan11_data; break;
		case 12: pointer = marathon_man_alan12_data; break;
		case 13: pointer = marathon_man_alan13_data; break;
		case 14: pointer = marathon_man_alan14_data; break;
		case 15: pointer = marathon_man_alan15_data; break;
		case 16: pointer = marathon_man_alan16_data; break;
		case 17: pointer = marathon_man_alan17_data; break;
		case 18: pointer = marathon_man_alan18_data; break;
		case 19: pointer = marathon_man_alan19_data; break;
		case 20: pointer = marathon_man_alan20_data; break;
		case 21: pointer = marathon_man_alan21_data; break;
		case 22: pointer = marathon_man_alan22_data; break;
		case 23: pointer = marathon_man_alan23_data; break;
		case 24: pointer = marathon_man_alan24_data; break;
		case 25: pointer = marathon_man_alan25_data; break;
		case 26: pointer = marathon_man_alan26_data; break;
		case 27: pointer = marathon_man_alan27_data; break;
		case 28: pointer = marathon_man_alan28_data; break;
		case 29: pointer = marathon_man_alan29_data; break;
		case 30: pointer = marathon_man_alan30_data; break;
		case 31: pointer = marathon_man_alan31_data; break;
		case 32: pointer = marathon_man_alan32_data; break;
		case 33: pointer = marathon_man_alan33_data; break;
		case 34: pointer = marathon_man_alan34_data; break;
		case 35: pointer = marathon_man_alan35_data; break;
		case 36: pointer = marathon_man_alan36_data; break;
		case 37: pointer = marathon_man_alan37_data; break;
		case 38: pointer = marathon_man_alan38_data; break;
		case 39: pointer = marathon_man_alan39_data; break;
		case 40: pointer = marathon_man_alan40_data; break;
		case 41: pointer = marathon_man_alan41_data; break;
		case 42: pointer = marathon_man_alan42_data; break;
		case 43: pointer = marathon_man_alan43_data; break;
		case 44: pointer = marathon_man_alan44_data; break;
		case 45: pointer = marathon_man_alan45_data; break;
		case 46: pointer = marathon_man_alan46_data; break;
		case 47: pointer = marathon_man_alan47_data; break;
		case 48: pointer = marathon_man_alan48_data; break;
		case 49: pointer = marathon_man_alan49_data; break;
		case 50: pointer = marathon_man_alan50_data; break;
		case 51: pointer = marathon_man_alan51_data; break;
		case 52: pointer = marathon_man_alan52_data; break;
		case 53: pointer = marathon_man_alan53_data; break;
		case 54: pointer = marathon_man_alan54_data; break; 
		default: pointer = marathon_man_alan54_data; break;
	}
		oam_meta_spr(40, 10, pointer);
}

void load_title(void){
	ppu_off();
	pal_bg(palette_title);
	pal_spr(palette_sprites);
	music_play(0);
	oam_clear();
	vram_adr(NAMETABLE_A);
	for (largeindex = 0; largeindex < 1024; ++largeindex)
	{
		vram_put(titlespecial[largeindex]);
		flush_vram_update2(); 
	}
	build_options_nametable(1);
	title_animation_frame = 0;
	title_frame_counter = 0;
	title_scroll_x = 0;
	title_runner_x = 24;
	title_transition_phase = 0;
	game_mode = MODE_TITLE;
	set_scroll_x(0);
	set_scroll_y(0);
	ppu_on_all();
}

void clear_background(void)
{
	unsigned char nt;

	// draw all 0x00 into the bg
	for(nt = 0; nt < 2; ++nt){
		vram_adr(NAMETABLE_A + ((unsigned int)nt << 10));
		for (largeindex = 0; largeindex < 1024; ++largeindex)
		{
			vram_put(0x00);
		}
	}
}

void load_room(){
	ppu_off();
	clear_background();
	init_world_sequence();
	screen_slots[0] = next_world_screen();
	screen_slots_compact[0] = 1;
	draw_full_room(screen_slots[0], screen_slots_compact[0], 0);
	// place a tile for sprite zero hit
	
	ppu_on_all();

}

void load_room_pair(void){
	ppu_off();
	clear_background();
	init_world_sequence();
	screen_slots[0] = next_world_screen();
	screen_slots[1] = next_world_screen();
	screen_slots_compact[0] = 1;
	screen_slots_compact[1] = 1;
	draw_full_room(screen_slots[0], screen_slots_compact[0], 0);
	draw_full_room(screen_slots[1], screen_slots_compact[1], 1);
	last_stream_column = 0xffff;
	stream_active = 0;
	stream_stage = 0;
}

void draw_screen_R(void){
	unsigned int column_progress;

	if((velocity == 0) && (stream_active == 0)){
		did_stream_column = 0;
		return;
	}

	did_stream_column = stream_column_chunk();
	if(stream_active != 0){
		if(stream_column_chunk() != 0){
			did_stream_column = 1;
		}
	}

	column_progress = scroll_x >> 3;
	if((column_progress == last_stream_column) || (stream_active != 0)){
		return;
	}
	last_stream_column = column_progress;

	stream_column = (unsigned char)((column_progress + 32u) & 31u);
	stream_room_index = (unsigned char)(((scroll_x >> 8) + 1u) & 1u);
	stream_nametable = (unsigned char)(((scroll_x >> 8) + 1u) & 1u);
	if(stream_column == 0){
		if(ending_sequence_active != 0){
			if(ending_grandstand_active != 0){
				screen_slots[stream_room_index] = grandstand;
			} else if(ending_queue_stage == 0){
				screen_slots[stream_room_index] = ending_forced_end_screen;
				ending_queue_stage = 1;
			} else if(ending_queue_stage == 1){
				screen_slots[stream_room_index] = trackbegin;
				ending_queue_stage = 2;
			} else {
				pal_bg(palette_grandstand);
				screen_slots[stream_room_index] = grandstand;
				ending_grandstand_active = 1;
			}
		} else {
			screen_slots[stream_room_index] = next_world_screen();
		}
		screen_slots_compact[stream_room_index] = 1;
	}
	stream_active = 1;
	stream_stage = 0;

	if(stream_column_chunk() != 0){
		did_stream_column = 1;
		if(stream_active != 0){
			stream_column_chunk();
		}
	}
}

void set_sprite_zero(void){
	oam_set(0); // double check that this goes in the zero slot
	
	//oam_spr(unsigned char x,unsigned char y,unsigned char chrnum,unsigned char attr);
	oam_spr(0xf0,70,0xfa,3 | OAM_BEHIND);
}