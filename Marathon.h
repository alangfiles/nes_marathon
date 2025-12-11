
#pragma bss-name(push, "ZEROPAGE")

#define FRAMES_PER_STEP 12
#define MOTION_HOLD_FRAMES 180

// GLOBAL VARIABLES
unsigned int powerpad_cur;
unsigned int powerpad_old; //don't use this
unsigned int powerpad_new;

unsigned int temp_int;

unsigned char last_step;

unsigned int steps;
unsigned char ones_step;
unsigned char tens_step;
unsigned char hundreds_step;
unsigned char thousands_step;
unsigned char ten_thousands_step;


unsigned int seconds;
unsigned char ones_seconds;
unsigned char tens_seconds;
unsigned char ones_minutes;
unsigned char tens_minutes;
unsigned char ones_hours;
unsigned char tens_hours;

unsigned char frame_counter;
unsigned char step_button_lockout;
unsigned char motion_counter;
unsigned char time_since_button_press;
unsigned char motion;
unsigned char was_running;
unsigned char was_walking;

unsigned int largeindex;

enum {
  RUNNING,
  WALKING,
  STANDING
};

void add_step(void);
void add_second(void);
void initial_steps_conversion(void);
void initial_timer_conversion(void);
void check_motion(void);
void update_motion(void);
void load_room(void);