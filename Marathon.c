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

	clear_vram_buffer();

	steps = 0;
	seconds = 0;
	scroll_x = 0;
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

			debug_controller = pad_poll(0);
			debug_controller_new = get_pad_new(0);

			if(debug_controller_new & PAD_START){
				init_options();
			}
		}

		while(game_mode == MODE_OPTIONS){
			ppu_wait_nmi();

			debug_controller = pad_poll(0);
			debug_controller_new = get_pad_new(0);

			if(debug_controller_new & PAD_UP){
				if(selected_option > 0) --selected_option;
				draw_options_screen();
			}
			if(debug_controller_new & PAD_DOWN){
				if(selected_option < 2) ++selected_option;
				draw_options_screen();
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
		++scroll_timer;
		if(time_since_button_press < 255){
			++time_since_button_press;
		}
		

		if(motion == RUNNING && scroll_timer >= 8){
			scroll_timer = 0;
			scroll_x += 2;
		} else if(motion == WALKING && scroll_timer >= 16){
			scroll_timer = 0;
			++scroll_x;
		} else if(motion == STANDING){
			scroll_timer = 0;
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
	// determine motion state from steps-per-minute calculated via frame timing
	if(time_since_button_press < 36) {
		// >100 steps per minute
		motion = RUNNING;
	} else if(time_since_button_press <= 120) {
		// 30-100 steps per minute
		motion = WALKING;
	} else {
		// <30 steps per minute
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
	oam_clear();
	set_sprite_zero();
	oam_set(4); // start drawing from slot 4

	++sprite_frame_counter;
	if(sprite_frame_counter >= 60){
		sprite_frame_counter = 0;
	}

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

	multi_vram_buffer_horz("  STEPS: ", 9, NTADR_A(1, 4));
	one_vram_buffer(0x30+ten_thousands_step, NTADR_A(10, 4));
	one_vram_buffer(0x30+thousands_step, NTADR_A(11, 4));
	one_vram_buffer(',', NTADR_A(12, 4));
	one_vram_buffer(0x30+hundreds_step, NTADR_A(13, 4));
	one_vram_buffer(0x30+tens_step, NTADR_A(14, 4));
	one_vram_buffer(0x30+ones_step, NTADR_A(15, 4));

	multi_vram_buffer_horz("  TIME:", 7, NTADR_A(1, 2));
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
	// Draw cursor ">" and spaces to deselect all rows first
	one_vram_buffer((selected_option == 0) ? '>' : ' ', NTADR_A(9, 10));
	one_vram_buffer((selected_option == 1) ? '>' : ' ', NTADR_A(9, 14));
	one_vram_buffer((selected_option == 2) ? '>' : ' ', NTADR_A(9, 18));
}

void init_options(void){
	ppu_off();
	pal_bg(palette_bg);
	pal_spr(palette_sprites);
	oam_clear();
	clear_vram_buffer();

	selected_option = 0;

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
	// Initial cursor
	vram_adr(NTADR_A(9, 10));
	vram_put('>');

	game_mode = MODE_OPTIONS;
	set_scroll_x(0);
	set_scroll_y(0);
	ppu_on_all();
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