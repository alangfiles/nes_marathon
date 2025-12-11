/*	example code for cc65, for NES
 *  test power pad
 *	Doug Fraker 2018
 */	

 
 
#include "LIB/neslib.h"
#include "LIB/nesdoug.h"
#include "LIB/padlib.h"


#include "Marathon.h"
#include "Track.h"


const unsigned char palette_sprites[16]={ 0x1b,0x26,0x07,0x21,0x1b,0x0c,0x21,0x32,0x1b,0x36,0x06,0x15,0x1b,0x27,0x16,0x29 };

const unsigned char palette_bg[16]={ 0x0f,0x30,0x07,0x21,0x0f,0x00,0x19,0x10,0x0f,0x17,0x06,0x19,0x0f,0x29,0x19,0x09 };


// do after the read
void process_powerpad(void){ 
	
	powerpad_new = (powerpad_cur^powerpad_old)&powerpad_cur;
	
	powerpad_old = powerpad_cur;
}	

	
	
void main (void) {
	
	ppu_off(); // screen off
	
	// load the palettes
	pal_bg(palette_bg);
	pal_spr(palette_sprites);

	// use the second set of tiles for sprites
	bank_spr(1);

	clear_vram_buffer();
	set_vram_buffer();
	//draw step count
	multi_vram_buffer_horz("STEPS:  00,000", 14, NTADR_A(2, 4));

	//and Timer
	multi_vram_buffer_horz("TIME: 00:00:00", 14, NTADR_A(2, 2));
	ppu_on_all(); // turn on screen


	// INITIALIZATION
	//setting these here so we can use gamegenie later
	steps = 0;
	seconds = 0;

	initial_steps_conversion();
	initial_timer_conversion();

	motion = STANDING;
	motion_counter = 0;
	multi_vram_buffer_horz("STAND", 5, NTADR_A(2, 6));

	//END INITIALIZATION
	
	

	load_room();
	
	while (1){
		// infinite loop

		ppu_wait_nmi(); // wait till beginning of the frame

		//timer stuff
		++frame_counter;
		if(time_since_button_press < 255){
			++time_since_button_press;
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

		oam_clear();
		
		powerpad_cur = read_powerpad(1);
		process_powerpad(); // goes after the read
							// transfers only new presses to powerpad_new
							// powerpad_new isn't used here, but
							// would be very useful for a game

		check_motion();
		if(motion_counter == 0){
			update_motion();
		}

		//if a new button is pressed down, we call it a step.
		if(powerpad_new & POWERPAD_1){
			oam_spr(84, 83, 0, 0);
			add_step();
		}
		
		if(powerpad_new & POWERPAD_2){
			oam_spr(100, 83, 0, 0);
			add_step();
		}
		
		if(powerpad_new & POWERPAD_3){
			oam_spr(116, 83, 0, 0);
			add_step();
		}
		
		if(powerpad_new & POWERPAD_4){
			oam_spr(132, 83, 0, 0);
			add_step();
		}
		
		if(powerpad_new & POWERPAD_5){
			oam_spr(84, 107, 0, 0);
			add_step();
		}
		
		if(powerpad_new & POWERPAD_6){
			oam_spr(100, 107, 0, 0);
			add_step();
		}
		
		if(powerpad_new & POWERPAD_7){
			oam_spr(116, 107, 0, 0);
			add_step();
		}
		
		if(powerpad_new & POWERPAD_8){
			oam_spr(132, 107, 0, 0);
			add_step();
		}
		
		if(powerpad_new & POWERPAD_9){
			oam_spr(84, 131, 0, 0);
			add_step();
		}
		
		if(powerpad_new & POWERPAD_10){
			oam_spr(100, 131, 0, 0);
			add_step();
		}
		
		if(powerpad_new & POWERPAD_11){
			oam_spr(116, 131, 0, 0);
			add_step();
		}
		
		if(powerpad_new & POWERPAD_12){
			oam_spr(132, 131, 0, 0);
			add_step();
		}
		
	}
}

void add_second(void){
	seconds++;
	
	if(ones_seconds == 9){
		ones_seconds = 0;
		one_vram_buffer(0x30+ones_seconds, NTADR_A(15, 2));
		if(tens_seconds == 5){
			tens_seconds = 0;
			one_vram_buffer(0x30+tens_seconds, NTADR_A(14, 2));
			if(ones_minutes == 9){
				ones_minutes = 0;
				one_vram_buffer(0x30+ones_minutes, NTADR_A(12, 2));
				if(tens_minutes == 5){
					tens_minutes = 0;
					one_vram_buffer(0x30+tens_minutes, NTADR_A(11, 2));
					if(ones_hours == 9){
						ones_hours = 0;
						one_vram_buffer(0x30+ones_hours, NTADR_A(9, 2));
						if(tens_hours == 2){
							tens_hours = 0;
							one_vram_buffer(0x30+tens_hours, NTADR_A(8, 2));
						} else {
							tens_hours++;
							one_vram_buffer(0x30+tens_hours, NTADR_A(8, 2));
						}
					} else {
						ones_hours++;
						one_vram_buffer(0x30+ones_hours, NTADR_A(9, 2));
					}
				} else {
					tens_minutes++;
					one_vram_buffer(0x30+tens_minutes, NTADR_A(11, 2));
				}
			} else {
				ones_minutes++;
				one_vram_buffer(0x30+ones_minutes, NTADR_A(12, 2));
			}
		} else {
			tens_seconds++;
			one_vram_buffer(0x30+tens_seconds, NTADR_A(14, 2));
		}
	} else {
		ones_seconds++;
		one_vram_buffer(0x30+ones_seconds, NTADR_A(15, 2));
	}
	
	
}

void add_step(void){
	if(step_button_lockout > 0){
		return; //still in lockout period
	}

	time_since_button_press = 0;
	steps++;
	step_button_lockout = FRAMES_PER_STEP; //lock out for a few frames to avoid double counting

	if(ones_step == 9){
		ones_step = 0;
		one_vram_buffer(0x30+ones_step, NTADR_A(15, 4));
		if(tens_step == 9){
			tens_step = 0;
			one_vram_buffer(0x30+tens_step, NTADR_A(14, 4));
			if(hundreds_step == 9){
				hundreds_step = 0;
				one_vram_buffer(0x30+hundreds_step, NTADR_A(13, 4));
				if(thousands_step == 9){
					thousands_step = 0;
					one_vram_buffer(0x30+thousands_step, NTADR_A(11, 4));
					if(ten_thousands_step == 9){
						ten_thousands_step = 0;
						one_vram_buffer(0x30+ten_thousands_step, NTADR_A(10, 4));
					} else {
						ten_thousands_step++;
						one_vram_buffer(0x30+ten_thousands_step, NTADR_A(10, 4));
					}
				} else {
					thousands_step++;
					one_vram_buffer(0x30+thousands_step, NTADR_A(11, 4));
				}
			} else {
				hundreds_step++;
				one_vram_buffer(0x30+hundreds_step, NTADR_A(13, 4));
			}
		} else {
			tens_step++;
			one_vram_buffer(0x30+tens_step, NTADR_A(14, 4));
		}
	} else {
		ones_step++;
		one_vram_buffer(0x30+ones_step, NTADR_A(15, 4));
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
	
	if(motion == WALKING){
		multi_vram_buffer_horz("WALK ", 5, NTADR_A(2, 6));
	}
	if(motion == RUNNING){
		multi_vram_buffer_horz("RUN  ", 5, NTADR_A(2, 6));
	}
	if(motion == STANDING){
		multi_vram_buffer_horz("STAND", 5, NTADR_A(2, 6));
	}
	

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


void load_room(){
	ppu_off();
	vram_adr(NAMETABLE_A);
	for (largeindex = 0; largeindex < 1024; ++largeindex)
	{
		vram_put(Track[largeindex]);
		flush_vram_update2();
	}
	ppu_on_all();

}