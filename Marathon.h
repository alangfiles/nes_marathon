
#pragma bss-name(push, "ZEROPAGE")

#define FRAMES_PER_STEP 10
#define MOTION_HOLD_FRAMES 180
#define TARGET_RESPAWN_FRAMES 240
#define TARGET_HIT_X 200
#define TARGET_START_X 248
#define TARGET_MAX_SPEED_STREAK 10
#define TARGET_BASE_SPEED_FP 205
#define TARGET_FAST_SPEED_FP 410
#define ENDING_SEQUENCE_STEPS 350u
#define FINAL_RUNOFF_STEPS 0u

// GLOBAL VARIABLES
unsigned int powerpad_cur;
unsigned int powerpad_old; //don't use this
unsigned int powerpad_new;

unsigned char debug_controller;
unsigned char debug_controller_new;

unsigned int temp_int;

unsigned char last_step;

unsigned int steps;
unsigned int score;
unsigned char score_to_add;
unsigned char ones_step;
unsigned char tens_step;
unsigned char hundreds_step;
unsigned char thousands_step;
unsigned char ten_thousands_step;
unsigned char ones_score;
unsigned char tens_score;
unsigned char hundreds_score;
unsigned char thousands_score;
unsigned char ten_thousands_score;

unsigned char target_button;
unsigned int target_mask;
unsigned char target_x;
unsigned int target_x_fp;
unsigned int target_speed_fp;
unsigned char target_spawn_timer;
unsigned int streak;
unsigned char hundreds_streak;
unsigned char tens_streak;
unsigned char ones_streak;
const unsigned char *target_sprite_data;

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
unsigned char title_scroll_x;
unsigned char title_runner_x;
unsigned char title_transition_phase;
unsigned char options_cursor_frame;
unsigned char options_cursor_timer;
unsigned char options_phase;
unsigned char options_runner_x;
unsigned char options_runner_timer;
unsigned char options_runner_frame;
unsigned char options_runner_hold_timer;
unsigned char game_intro_timer;
unsigned char ending_sequence_active;
unsigned char ending_queue_stage;
unsigned char ending_grandstand_active;
unsigned char ending_runoff_active;
unsigned char runner_screen_x;
const unsigned char *ending_forced_end_screen;

unsigned int scroll_x;
unsigned int scroll_subpixel;
unsigned int velocity;
unsigned int temp_int;
unsigned int largeindex;
const unsigned char *pointer;
unsigned char scroll_timer;
unsigned int total_steps_needed;
unsigned int progress_remainder;
unsigned char progress_pixels;
unsigned int last_stream_column;
unsigned char column_buffer[30];
unsigned char attribute_buffer[8];
unsigned char stream_active;
unsigned char stream_room_index;
unsigned char stream_nametable;
unsigned char stream_column;
unsigned char stream_stage;
unsigned char did_stream_column;

unsigned char game_mode;
unsigned char selected_option;
unsigned char race_type;
enum {MODE_TITLE, MODE_TITLE_TO_OPTIONS, MODE_OPTIONS, MODE_GAME_INTRO, MODE_GAME, MODE_WIN};
enum {RACE_5K, RACE_10K, RACE_MARATHON};
enum {OPTIONS_PHASE_IDLE, OPTIONS_PHASE_CONFIRM};

enum {
  RUNNING,
  WALKING,
  STANDING
};

void add_step(void);
void add_score(void);
void add_second(void);
void initial_steps_conversion(void);
void initial_timer_conversion(void);
void check_motion(void);
void update_motion(void);
void load_room(void);
void draw_sprite(void);
void draw_hud(void);
void process_controller(void);
void init_target_button(void);
void spawn_target_button(void);
void update_target_button(void);
void draw_target_button(void);
void reset_streak(void);
void add_streak_hit(void);
unsigned int get_target_speed_fp(void);
const unsigned char *get_big_button_sprite(unsigned char button);
unsigned int get_powerpad_button_mask(unsigned char button);
unsigned char random_button_value(void);
void set_sprite_zero(void);
void update_steps_per_minute(void);
void load_title(void);
void init_mode_game(void);
void init_options(void);
void init_win_screen(void);
void draw_options_screen(void);
void set_signature_sprite(void);
void load_room_pair(void);
void draw_screen_R(void);
void build_options_nametable(unsigned char nametable);
void begin_title_to_options(void);
void draw_title_to_options(void);
const unsigned char *get_end_screen_for_room(const unsigned char *room_data);
void activate_grandstand_section(void);
void update_endgame_state(void);
