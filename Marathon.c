/*	example code for cc65, for NES
 *  test power pad
 *	Doug Fraker 2018
 */	

 
 
#include "LIB/neslib.h"
#include "LIB/nesdoug.h"
#include "LIB/padlib.h"


#include "Marathon.h"
#include "Track.h"
#include "sprites.h"

const unsigned char palette_sprites[16]={
	 0x1b,0x26,0x07,0x21,
	 0x1b,0x0c,0x21,0x32,
	 0x1b,0x36,0x06,0x15,
	 0x1b,0x27,0x16,0x29 };

const unsigned char palette_bg[16]={ 
	0x0f,0x30,0x07,0x21,
	0x0f,0x00,0x19,0x10,
	0x0f,0x17,0x06,0x19,
	0x0f,0x29,0x19,0x09 };


// do after the read
void process_powerpad(void){ 
	
	powerpad_new = (powerpad_cur^powerpad_old)&powerpad_cur;
	
	powerpad_old = powerpad_cur;
}	

	
	
void main (void) {

	clear_vram_buffer(); 
	set_vram_buffer();
	set_scroll_y(0xff); // shift the bg down one pixel
	
	ppu_off(); // screen off
	
	// load the palettes
	pal_bg(palette_bg); 
	pal_spr(palette_sprites);

	// use the second set of tiles for sprites
	bank_spr(1);
	load_room();
	
	set_sprite_zero();	// this needs to be done before ppu_on_all, to make sure the sprite zero is in place for the split

	ppu_on_all(); // turn on screen


	// INITIALIZATION
	//setting these here so we can use gamegenie later
	steps = 0;
	seconds = 0;

	initial_steps_conversion();
	initial_timer_conversion();

	motion = STANDING;
	motion_counter = 0;

	//END INITIALIZATION
	
	

	
	
	while (1){
		// infinite loop

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
		

		if(scroll_timer >= 8){
			scroll_timer = 0;
			++scroll_x ; //debug no scrolling
		}
		

		if(step_button_lockout > 0){
			--step_button_lockout;
		}

		if(motion_counter > 0){
			--motion_counter;
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
		if(motion_counter == 0){
			update_motion();
		}

		draw_sprite();
		draw_hud();

		process_controller();

		
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
	// are two buttons being pressed currently? if so, then we're walking for 180 frames.
	if((powerpad_cur & 0x0F) && (powerpad_cur & 0xF0) && time_since_button_press < 180){
		was_walking = 1;
	}else if(powerpad_cur == 0 && time_since_button_press < 60){ //nothing is pressed
		was_running = 1;
	} else {

	}
}

void update_motion(void){
	// called when motion_counter times out
	if(was_running){
		motion = RUNNING;
		motion_counter = MOTION_HOLD_FRAMES;
	}
	else if(was_walking){
		motion = WALKING;
		motion_counter = MOTION_HOLD_FRAMES;
	}
	else {
		motion = STANDING;
	}
	was_walking = 0;
	was_running = 0;
	

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
	
	//draw the player sprite based on motion type
	++sprite_frame_counter;

	if(sprite_frame_counter <10){
		oam_meta_spr(120, 120, running_man_3_data);	
	} else if (sprite_frame_counter <20){
		oam_meta_spr(120, 120, running_man_5_data);
	} else if (sprite_frame_counter <30){
		oam_meta_spr(120, 120, running_man_6_data);
	} else if (sprite_frame_counter <40){
		oam_meta_spr(120, 120, running_man_7_data);
	} else if (sprite_frame_counter <50){
		oam_meta_spr(120, 120, running_man_1_data);
	} else if (sprite_frame_counter < 59){
		oam_meta_spr(120, 120, running_man_2_data);
	} else {
		sprite_frame_counter = 0;
		oam_meta_spr(120, 120, running_man_2_data);
	}


}


void load_room(){
	ppu_off();
	vram_adr(NAMETABLE_A);
	for (largeindex = 0; largeindex < 1024; ++largeindex)
	{ 
		vram_put(track[largeindex]);
		flush_vram_update2();
	}
	// place a tile for sprite zero hit at x=120, y=64
	vram_adr(NTADR_A(15,8));
	vram_put(0x69);
	ppu_on_all();

}

void set_sprite_zero(void){
	oam_set(0); // double check that this goes in the zero slot
	
	//oam_spr(unsigned char x,unsigned char y,unsigned char chrnum,unsigned char attr);
	oam_spr(0x78,0x40,0x69,3); // x=120, y=64, tile=105, attr=3
}