enum blink_modes {BRIGHT, BLINK, FLUID};
enum source_modes {POT, SER};
extern enum blink_modes blink_mode;
extern volatile enum source_modes source_mode;

extern volatile uint8_t pot_val;
extern volatile uint8_t scaled_time;

extern void calculate_timers(uint8_t pot, uint8_t sc_time);
extern void led_state_get();
extern void led_state_set(char mode);
extern void led_value_set(uint8_t ac, char *av);
extern void set_led();