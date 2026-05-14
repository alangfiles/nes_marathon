/*	example code for cc65, for NES
 *  test power pad
 *	Doug Fraker 2018
 */	

 
 
#include "LIB/neslib.h"
#include "LIB/nesdoug.h"
#include "LIB/padlib.h"


#include "Marathon.h"
#include "SCREENS/trackflowers.h"
#include "SCREENS/title.h"
#include "sprites.h"

const unsigned char palette_sprites[16]={
	 0x0f,0x26,0x07,0x21,
	 0x0f,0x30,0x15,0x11,
	 0x0f,0x36,0x07,0x15,
	 0x0f,0x17,0x07,0x0f };

const unsigned char palette_bg[16]={ 
	0x0f,0x2d,0x1a,0x10,
	0x0f,0x11,0x1a,0x21,
	0x0f,0x2a,0x1a,0x0a,
	0x0f,0x17,0x1a,0x07 };


// do after the read
void process_powerpad(void){ 
	
	powerpad_new = (powerpad_cur^powerpad_old)&powerpad_cur;
	
	powerpad_old = powerpad_cur;
}	

unsigned int get_powerpad_button_mask(unsigned char button){
	switch(button){
		case 1: return POWERPAD_1;
		case 2: return POWERPAD_2;
		case 3: return POWERPAD_3;
		case 4: return POWERPAD_4;
		case 5: return POWERPAD_5;
		case 6: return POWERPAD_6;
		case 7: return POWERPAD_7;
		case 8: return POWERPAD_8;
		case 9: return POWERPAD_9;
		case 10: return POWERPAD_10;
		case 11: return POWERPAD_11;
		case 12: return POWERPAD_12;
		default: return 0;
	}
}

unsigned char random_button_value(void){
	unsigned char value;

	value = rand8() & 0x0f;
	if(value >= 12){
		value -= 12;
	}
	return value + 1;
}

const unsigned char *get_big_button_sprite(unsigned char button){
	switch(button){
		case 1: return marathon_man_1big_data;
		case 2: return marathon_man_2big_data;
		case 3: return marathon_man_3big_data;
		case 4: return marathon_man_4big_data;
		case 5: return marathon_man_5big_data;
		case 6: return marathon_man_6big_data;
		case 7: return marathon_man_7big_data;
		case 8: return marathon_man_8big_data;
		case 9: return marathon_man_9big_data;
		case 10: return marathon_man_10big_data;
		case 11: return marathon_man_11big_data;
		default: return marathon_man_12big_data;
	}
}

void update_streak_digits(void){
	temp_int = streak;
	ones_streak = temp_int % 10;
	temp_int /= 10;
	tens_streak = temp_int % 10;
	temp_int /= 10;
	hundreds_streak = temp_int % 10;
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
	264, 319, 374, 429, 483, 539, 593, 648, 702, 758, 813
};

unsigned int get_target_speed_fp(void){
	unsigned char idx = (unsigned char)streak;
	if(idx > 10){
		idx = 10;
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
		reset_streak();
		spawn_target_button();
	}
}

void add_score(void){
	
	score += score_to_add;

	while(score_to_add > 0){
		--score_to_add;
		if(ones_score == 9){
			ones_score = 0;
			if(tens_score == 9){
				tens_score = 0;
				if(hundreds_score == 9){
					hundreds_score = 0;
					if(thousands_score == 9){
						thousands_score = 0;
						if(ten_thousands_score < 9){
							++ten_thousands_score;
						}
					}else {
						++thousands_score;
					}
				} else {
					++hundreds_score;
				}
			} else {
				++tens_score;
			}
		} else {
			++ones_score;
		}
	}
}

void draw_target_button(void){
	if(target_x > 8){
		oam_meta_spr(target_x, 88, target_sprite_data);
	}
}

void init_hud_labels(void){
	vram_adr(NTADR_A(2, 4));
	vram_put('S'); vram_put('T'); vram_put('E'); vram_put('P'); vram_put('S'); vram_put(':');

	vram_adr(NTADR_A(2, 2));
	vram_put('T'); vram_put('I'); vram_put('M'); vram_put('E'); vram_put(':');

	vram_adr(NTADR_A(18, 2));
	vram_put('S'); vram_put('C'); vram_put('O'); vram_put('R'); vram_put('E'); vram_put(':');

	vram_adr(NTADR_A(18, 4));
	vram_put('S'); vram_put('T'); vram_put('R'); vram_put('E');
	vram_put('A'); vram_put('K'); vram_put(':');
}


void init_mode_game(void){
	ppu_off();
	pal_bg(palette_bg);
	pal_spr(palette_sprites);


	//set race type
	race_type = selected_option;
	if(race_type == RACE_5K){
		total_steps_needed = 6000u; 
	} else if (race_type == RACE_10K){
		total_steps_needed = 12000u;
	} else {
		total_steps_needed = 52000u;
	}

	clear_vram_buffer();

	// mess with these for debug, but they should all be 0
	steps = 0;
	score = 0;
	seconds = 0;

	scroll_x = 0;
	scroll_subpixel = 0;
	velocity = 0; 
	frame_counter = 0;
	scroll_timer = 0;
	step_button_lockout = 0;
	sprite_frame_counter = 0;
	time_since_button_press = 255;
	motion = STANDING;
	powerpad_old = 0;
	powerpad_new = 0;
	ones_score = 0;
	tens_score = 0;
	hundreds_score = 0;
	thousands_score = 0;
	ten_thousands_score = 0;
	reset_streak();
	init_target_button();

	initial_steps_conversion();
	initial_timer_conversion();

	load_room();
	ppu_off();
	init_hud_labels();
	set_sprite_zero();
	// set_scroll_x(0);
	set_scroll_y(0xff);

	game_mode = MODE_GAME;
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

			debug_controller = pad_poll(0);
			debug_controller_new = get_pad_new(0);

			if(debug_controller_new & PAD_START){
				init_options();
			}
		}

		while(game_mode == MODE_OPTIONS){
			ppu_wait_nmi();
			draw_options_screen();

			debug_controller = pad_poll(0);
			debug_controller_new = get_pad_new(0);

			if(debug_controller_new & PAD_UP){
				if(selected_option > 0) --selected_option;
			}
			if(debug_controller_new & PAD_DOWN){
				if(selected_option < 2) ++selected_option;
			}
			if(debug_controller_new & PAD_START || debug_controller_new & PAD_A){
				race_type = selected_option;
				init_mode_game();
			}
		}

		while(game_mode == MODE_GAME){

		ppu_wait_nmi(); // wait till beginning of the frame
		
		set_sprite_zero();
		split(scroll_x);
		set_scroll_x(0);
		set_scroll_y(0xff);

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

		// Accumulate subpixel camera movement so low velocity still scrolls.
		scroll_subpixel += velocity;
		scroll_x = (scroll_subpixel >> 8);
		

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

		draw_sprite();
		draw_hud();

		process_controller();

		if(steps >= total_steps_needed){
			init_win_screen();
			break;
		}

		
                } // end MODE_GAME

		while(game_mode == MODE_WIN){
			ppu_wait_nmi();
			oam_clear();

			debug_controller = pad_poll(0);
			debug_controller_new = get_pad_new(0);

			if((debug_controller_new & PAD_START) || (debug_controller_new & PAD_A)){
				load_title();
			}
		}

        } // end while(1)
}

void process_controller(void){
	if(target_mask != 0 && (powerpad_new & target_mask) && target_x <= TARGET_HIT_X){
		score_to_add = (unsigned char)(1u + streak);
		add_score();
		add_streak_hit();
		spawn_target_button();
	}

	if(debug_controller_new & PAD_A || debug_controller_new & PAD_B){
		add_step();
	}
	//if a new button is pressed down, we call it a step.
		if(powerpad_new & POWERPAD_1){
			add_step();
		}
		
		if(powerpad_new & POWERPAD_2){
			
			add_step();
		}
		
		if(powerpad_new & POWERPAD_3){
			
			add_step();
		}
		
		if(powerpad_new & POWERPAD_4){
			
			add_step();
		}
		
		if(powerpad_new & POWERPAD_5){
			
			add_step();
		}
		
		if(powerpad_new & POWERPAD_6){
			
			add_step();
		}
		
		if(powerpad_new & POWERPAD_7){
			
			add_step();
		}
		
		if(powerpad_new & POWERPAD_8){
			
			add_step();
		}
		
		if(powerpad_new & POWERPAD_9){
			
			add_step();
		}
		
		if(powerpad_new & POWERPAD_10){
			
			add_step();
		}
		
		if(powerpad_new & POWERPAD_11){
			
			add_step();
		}
		
		if(powerpad_new & POWERPAD_12){
			
			add_step();
		}
		
}

void add_second(void){
	seconds++;
	
	if(ones_seconds == 9){
		ones_seconds = 0;

		if(tens_seconds == 5){
			tens_seconds = 0;

			if(ones_minutes == 9){
				ones_minutes = 0;

				if(tens_minutes == 5){
					tens_minutes = 0;

					if(ones_hours == 9){
						ones_hours = 0;

						if(tens_hours == 2){
							tens_hours = 0;

						} else {
							tens_hours++;

						}
					} else {
						ones_hours++;

					}
				} else {
					tens_minutes++;

				}
			} else {
				ones_minutes++;

			}
		} else {
			tens_seconds++;

		}
	} else {
		ones_seconds++;
	}
	
	 
}

void add_step(void){
	
	if(step_button_lockout > 0){
		return; //still in lockout period
	}
	update_steps_per_minute(); // calculate SPM before resetting the timer
	sprite_timer = 0; //used for animation
	velocity += 100;
	if(velocity > 720){
		velocity = 720;
	}

	time_since_button_press = 0;
	steps++;
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
	ones_spm = temp_int % 10;
	temp_int /= 10;
	tens_spm = temp_int % 10;
	temp_int /= 10;
	hundreds_spm = temp_int % 10;
}

void initial_steps_conversion(void){
	
	temp_int = steps;
	
	ones_step = temp_int % 10;
	temp_int = temp_int / 10;
	
	tens_step = temp_int % 10;
	temp_int = temp_int / 10;
	
	hundreds_step = temp_int % 10;
	temp_int = temp_int / 10;
	
	thousands_step = temp_int % 10;
	temp_int = temp_int / 10;
	
	ten_thousands_step = temp_int % 10;

	one_vram_buffer(0x30+ten_thousands_step, NTADR_A(10, 4));
	one_vram_buffer(0x30+thousands_step, NTADR_A(11, 4));
	one_vram_buffer(0x30+hundreds_step, NTADR_A(13, 4));
	one_vram_buffer(0x30+tens_step, NTADR_A(14, 4));
	one_vram_buffer(0x30+ones_step, NTADR_A(15, 4));
}

void initial_timer_conversion(void){
	
	temp_int = seconds;
	
	ones_seconds = temp_int % 10;
	temp_int = temp_int / 10;
	
	tens_seconds = temp_int % 6;
	temp_int = temp_int / 6;
	
	ones_minutes = temp_int % 10;
	temp_int = temp_int / 10;
	
	tens_minutes = temp_int % 6;
	temp_int = temp_int / 6;
	
	ones_hours = temp_int % 10;
	temp_int = temp_int / 10;
	
	tens_hours = temp_int % 3;

	one_vram_buffer(0x30+tens_hours, NTADR_A(8, 2));
	one_vram_buffer(0x30+ones_hours, NTADR_A(9, 2));
	one_vram_buffer(0x30+tens_minutes, NTADR_A(11, 2));
	one_vram_buffer(0x30+ones_minutes, NTADR_A(12, 2));
	one_vram_buffer(0x30+tens_seconds, NTADR_A(14, 2));
	one_vram_buffer(0x30+ones_seconds, NTADR_A(15, 2));

}

void draw_sprite(){
	unsigned char progress_x;
	unsigned int clamped_steps;
	unsigned long progress_scaled;
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

	progress_x = 16;
	if(total_steps_needed > 0){
		clamped_steps = steps;
		if(clamped_steps > total_steps_needed){
			clamped_steps = total_steps_needed;
		}
		progress_scaled = ((unsigned long)clamped_steps * 212UL) / (unsigned long)total_steps_needed;
		progress_x = 16 + (unsigned char)progress_scaled;
	}

	// Draw progress cursor on the top race bar.
	oam_meta_spr(progress_x + 5, 74, progress_cursor_data);
	draw_target_button();

	if(motion == RUNNING){
		// 6 frames, 10 ticks each = 60-frame cycle
		if(sprite_frame_counter < 10){
			oam_meta_spr(120, 120, marathon_man_run1_data);
		} else if(sprite_frame_counter < 20){
			oam_meta_spr(120, 120, marathon_man_run2_data);
		} else if(sprite_frame_counter < 30){
			oam_meta_spr(120, 120, marathon_man_run3_data);
		} else if(sprite_frame_counter < 40){
			oam_meta_spr(120, 120, marathon_man_run4_data);
		} else if(sprite_frame_counter < 50){
			oam_meta_spr(120, 120, marathon_man_run5_data);
		} else {
			oam_meta_spr(120, 120, marathon_man_run6_data);
		}
	} else if(motion == WALKING){
		// 4 frames, 15 ticks each = 60-frame cycle
		if(sprite_frame_counter < 15){
			oam_meta_spr(120, 120, marathon_man_walk1_data);
		} else if(sprite_frame_counter < 30){
			oam_meta_spr(120, 120, marathon_man_walk2_data);
		} else if(sprite_frame_counter < 45){
			oam_meta_spr(120, 120, marathon_man_walk3_data);
		} else {
			oam_meta_spr(120, 120, marathon_man_walk4_data);
		}
	} else {
		// STANDING - hold a neutral walking pose
		oam_meta_spr(120, 120, marathon_man_walk1_data);
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
		cursor_y = 80;
	} else if(selected_option == 1){
		cursor_y = 112;
	} else {
		cursor_y = 144;
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
	oam_meta_spr(68, cursor_y - 2, cursor_data);
}

void init_options(void){
	ppu_off();
	pal_bg(palette_bg);
	pal_spr(palette_sprites);
	oam_clear();
	clear_vram_buffer();

	selected_option = 0;
	options_cursor_frame = 0;
	options_cursor_timer = 0;

	// Write blank nametable
	vram_adr(NAMETABLE_A);
	for(largeindex = 0; largeindex < 1024; ++largeindex){
		vram_put(0x00);
		flush_vram_update2();
	}

	// Draw option labels directly into VRAM while PPU is off
	// "5K" at row 10
	vram_adr(NTADR_A(11, 10));
	vram_put('5'); vram_put('K');
	// "10K" at row 14
	vram_adr(NTADR_A(11, 14));
	vram_put('1'); vram_put('0'); vram_put('K');
	// "MARATHON" at row 18
	vram_adr(NTADR_A(11, 18));
	vram_put('M'); vram_put('A'); vram_put('R'); vram_put('A');
	vram_put('T'); vram_put('H'); vram_put('O'); vram_put('N');

	game_mode = MODE_OPTIONS;
	set_scroll_x(0);
	set_scroll_y(0);
	ppu_on_all();
}

void init_win_screen(void){
	ppu_off();
	oam_clear();
	clear_vram_buffer();

	vram_adr(NAMETABLE_A);
	vram_fill(0x00, 1024);

	vram_adr(NTADR_A(11, 12));
	vram_put('Y'); vram_put('O'); vram_put('U');
	vram_put(' ');
	vram_put('W'); vram_put('I'); vram_put('N');

	vram_adr(NTADR_A(10, 15));
	vram_put('T'); vram_put('I'); vram_put('M'); vram_put('E');
	vram_put(':');
	vram_put(0x30 + tens_hours);
	vram_put(0x30 + ones_hours);
	vram_put(':');
	vram_put(0x30 + tens_minutes);
	vram_put(0x30 + ones_minutes);
	vram_put(':');
	vram_put(0x30 + tens_seconds);
	vram_put(0x30 + ones_seconds);

	vram_adr(NTADR_A(6, 20));
	vram_put('P'); vram_put('R'); vram_put('E'); vram_put('S'); vram_put('S');
	vram_put(' ');
	vram_put('S'); vram_put('T'); vram_put('A'); vram_put('R'); vram_put('T');

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
	pal_bg(palette_bg);
	pal_spr(palette_sprites);
	oam_clear();
	vram_adr(NAMETABLE_A);
	for (largeindex = 0; largeindex < 1024; ++largeindex)
	{
		vram_put(title[largeindex]);
		flush_vram_update2();
	}
	title_animation_frame = 0;
	title_frame_counter = 0;
	game_mode = MODE_TITLE;
	set_scroll_x(0);
	set_scroll_y(0);
	ppu_on_all();
}

void clear_background(void)
{
	// draw all 0x00 into the bg
	vram_adr(NAMETABLE_A);
	for (largeindex = 0; largeindex < 1024; ++largeindex)
	{
		vram_put(0x00);
		flush_vram_update2();
	}
}

void load_room(){
	ppu_off();
	clear_background();
	vram_adr(NAMETABLE_A);
	for (largeindex = 0; largeindex < 1024; ++largeindex)
	{ 
		vram_put(trackflowers[largeindex]);
		++index;
		if (index > 1)
		{ // don't put too much in the vram_buffer
			flush_vram_update2();
			index = 0;
		}
	}
	// place a tile for sprite zero hit
	vram_adr(NTADR_A(01,11));
	vram_put(0x01);
	ppu_on_all();

}

void set_sprite_zero(void){
	oam_set(0); // double check that this goes in the zero slot
	
	//oam_spr(unsigned char x,unsigned char y,unsigned char chrnum,unsigned char attr);
	oam_spr(0x01,88,0x02,3);
}