#ifndef _Leds_H
#define _Leds_H

#include "libs/Kernel.h"

#define leds_checksum     CHECKSUM("leds")

class Leds : public Module {
public:
    Leds();
    ~Leds();

    void on_module_loaded();
    void on_config_reload(void*);
    void on_main_init(void*);
    void on_sd_ok(void*);
    void on_main_loop(void*);
    void on_idle(void*);
    void on_gcode_received(void*);

private:
    int16_t     counter_main;
    int16_t     counter_idle;
    int16_t     counter_gcode;
};

#endif /* _Leds_H */
