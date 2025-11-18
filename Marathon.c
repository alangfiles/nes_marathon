/*	example code for cc65, for NES
 *  test power pad
 *	Doug Fraker 2018
 */	

 
 
#include "LIB/neslib.h"
#include "LIB/nesdoug.h"
#include "LIB/padlib.h"

#include "NES_ST/Marathon.h"
#include "Marathon.h"



const unsigned char pal1[]={
0x0f, 0x11, 0x16, 0x30,
0x0f, 0x00, 0x10, 0x30,
0,0,0,0,
0,0,0,0
}; 

const unsigned char pal2[]={
0x0f, 0x0f, 0x0f, 0x0f, // black
0,0,0,0,
0,0,0,0,
0,0,0,0
}; 



// do after the read
void process_powerpad(void){
	
	powerpad_new = (powerpad_cur^powerpad_old)&powerpad_cur;
	
	powerpad_old = powerpad_cur;
}	

	
	
void main (void) {
	
	ppu_off(); // screen off
	
	// load the palettes
	pal_bg(pal1);
	pal_spr(pal2);

	// use the second set of tiles for sprites
	bank_spr(1);

	vram_adr(NAMETABLE_A); // set a start position for the text
	vram_unrle(PowerBG);

	clear_vram_buffer();
	set_vram_buffer();
	//draw step count
	multi_vram_buffer_horz("STEPS:  00,000", 14, NTADR_A(2, 4));

	//and Timer
	multi_vram_buffer_horz("TIME: 00:00:00", 14, NTADR_A(2, 2));
	ppu_on_all(); // turn on screen


	steps = 1232;
	seconds = 521;

	initial_steps_conversion();
	initial_timer_conversion();
	

	
	while (1){
		// infinite loop

		ppu_wait_nmi(); // wait till beginning of the frame
		++frame_counter;

		if(frame_counter >= 60){
			frame_counter = 0;
			add_second();
		}

		oam_clear();
		
		powerpad_cur = read_powerpad(1);
		process_powerpad(); // goes after the read
							// transfers only new presses to powerpad_new
							// powerpad_new isn't used here, but
							// would be very useful for a game

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


		//draw steps from an int into 0-9 for each digit, without division
		//draw_steps();


		

		
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
	steps++;

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