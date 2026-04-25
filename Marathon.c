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
	 0x1b,0x26,0x07,0x21,
	 0x1b,0x0c,0x21,0x32,
	 0x1b,0x36,0x06,0x15,
	 0x1b,0x27,0x16,0x29 };

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


void init_mode_game(void){
	ppu_off();
	pal_bg(palette_bg);
	pal_spr(palette_sprites);


	//set race type
	race_type = selected_option;
	if(race_type == RACE_5K){
		total_steps_needed = 6000; 
	} else if (race_type == RACE_10K){
		total_steps_needed = 12000;
	} else {
		total_steps_needed = 52000;
	}

	clear_vram_buffer();

	steps = 0;
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

	initial_steps_conversion();
	initial_timer_conversion();

	load_room();
	ppu_off();
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

		check_motion();

		draw_sprite();
		draw_hud();

		process_controller();

		
                } // end MODE_GAME
        } // end while(1)
}

void process_controller(void){
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

	multi_vram_buffer_horz("@STEPS:@@", 9, NTADR_A(1, 4));
	one_vram_buffer(0x30+ten_thousands_step, NTADR_A(10, 4));
	one_vram_buffer(0x30+thousands_step, NTADR_A(11, 4));
	one_vram_buffer(',', NTADR_A(12, 4));
	one_vram_buffer(0x30+hundreds_step, NTADR_A(13, 4));
	one_vram_buffer(0x30+tens_step, NTADR_A(14, 4));
	one_vram_buffer(0x30+ones_step, NTADR_A(15, 4));

	multi_vram_buffer_horz("@TIME:@", 7, NTADR_A(1, 2));
	one_vram_buffer(0x30+tens_hours, NTADR_A(8, 2));
	one_vram_buffer(0x30+ones_hours, NTADR_A(9, 2));
	one_vram_buffer(':', NTADR_A(10, 2));
	one_vram_buffer(0x30+tens_minutes, NTADR_A(11, 2));
	one_vram_buffer(0x30+ones_minutes, NTADR_A(12, 2));
	one_vram_buffer(':', NTADR_A(13, 2));
	one_vram_buffer(0x30+tens_seconds, NTADR_A(14, 2));
	one_vram_buffer(0x30+ones_seconds, NTADR_A(15, 2));
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
	vram_adr(NTADR_A(10, 10));
	vram_put('5'); vram_put('K');
	// "10K" at row 14
	vram_adr(NTADR_A(10, 14));
	vram_put('1'); vram_put('0'); vram_put('K');
	// "MARATHON" at row 18
	vram_adr(NTADR_A(10, 18));
	vram_put('M'); vram_put('A'); vram_put('R'); vram_put('A');
	vram_put('T'); vram_put('H'); vram_put('O'); vram_put('N');

	game_mode = MODE_OPTIONS;
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