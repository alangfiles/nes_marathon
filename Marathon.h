
#pragma bss-name(push, "ZEROPAGE")

#define FRAMES_PER_STEP 10
#define MOTION_HOLD_FRAMES 180

// GLOBAL VARIABLES
unsigned int powerpad_cur;
unsigned int powerpad_old; //don't use this
unsigned int powerpad_new;

unsigned char debug_controller;
unsigned char debug_controller_new;

unsigned int temp_int;

unsigned char last_step;

unsigned int steps;
unsigned char ones_step;
unsigned char tens_step;
unsigned char hundreds_step;
unsigned char thousands_step;
unsigned char ten_thousands_step;

unsigned int steps_per_minute;
unsigned char hundreds_spm;
unsigned char tens_spm;
unsigned char ones_spm;


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
unsigned char index;

unsigned char mph;

unsigned char sprite_timer;
unsigned char sprite_frame_counter;

unsigned char title_animation_frame;
unsigned char title_frame_counter;
unsigned char options_cursor_frame;
unsigned char options_cursor_timer;

unsigned int scroll_x;
unsigned int scroll_subpixel;
unsigned int velocity;
unsigned int temp_int;
unsigned int largeindex;
const unsigned char *pointer;
unsigned char scroll_timer;
unsigned int total_steps_needed;

unsigned char game_mode;
unsigned char selected_option;
unsigned char race_type;
enum {MODE_TITLE, MODE_OPTIONS, MODE_GAME, MODE_WIN};
enum {RACE_5K, RACE_10K, RACE_MARATHON};

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
void draw_sprite(void);
void draw_hud(void);
void process_controller(void);
void set_sprite_zero(void);
void update_steps_per_minute(void);
void load_title(void);
void init_mode_game(void);
void init_options(void);
void init_win_screen(void);
void draw_options_screen(void);
void set_signature_sprite(void);
