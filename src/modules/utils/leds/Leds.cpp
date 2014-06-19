#include "Leds.h"

#include "Config.h"
#include "ConfigValue.h"
#include "checksumm.h"
#include "utils.h"
#include "libs/gpio.h"
#include "SlowTicker.h"
#include "Pauser.h"
#include "modules/robot/Conveyor.h"

#define enable_checksum         CHECKSUM("enable")
#define leds_disable_checksum   CHECKSUM("leds_disable")

#define led_gcode_checksum      CHECKSUM("led_gcode")
#define led_main_checksum       CHECKSUM("led_main")
#define led_idle_checksum       CHECKSUM("led_idle")
#define led_init_checksum       CHECKSUM("led_init")
#define led_sdok_checksum       CHECKSUM("led_sdok")
#define led_play_checksum       CHECKSUM("led_play")
#define led_main_mode_checksum  CHECKSUM("led_main_mode")
#define led_idle_mode_checksum  CHECKSUM("led_idle_mode")
#define blink_checksum          CHECKSUM("blink")
#define dimmed_checksum         CHECKSUM("dimmed")

#define MODE_BLINK              'b'
#define MODE_DIMMED             'd'

GPIO leds[] = {
    GPIO(P1_18),
    GPIO(P1_19),
    GPIO(P1_20),
    GPIO(P1_21),
    GPIO(P4_28)
};

const int n_leds = sizeof(leds)/sizeof(GPIO);


Leds::Leds()  {}
Leds::~Leds() {}

void Leds::on_module_loaded()
{
    // old config leds_disable
    bool use_leds = ! THEKERNEL->config->value( leds_disable_checksum )->by_default(false)->as_bool();
         use_leds =   THEKERNEL->config->value( leds_checksum, enable_checksum )->by_default( use_leds )->as_bool();

    // Exit if this module is not enabled
    if ( ! use_leds ) {
        delete this;
        return;
    }

    // Configuration
    this->on_config_reload(this);

    // register events after initializing scheme
    register_for_event(ON_CONFIG_RELOAD);
    register_for_event(ON_MAIN_INIT);
    register_for_event(ON_SD_OK);
    register_for_event(ON_MAIN_LOOP);
    register_for_event(ON_IDLE);
    register_for_event(ON_GCODE_RECEIVED);

    THEKERNEL->slow_ticker->attach(4, this, &Leds::half_second_tick);
}

void Leds::on_config_reload(void* argument)
{
    // Default pins to low status
    for (int i = 0; i < n_leds; i++){
        leds[i].output();
        leds[i]= 0;
    }

    led_init       = THEKERNEL->config->value( leds_checksum, led_init_checksum  )->by_default(1)->as_int() - 1;
    led_sdok       = THEKERNEL->config->value( leds_checksum, led_sdok_checksum  )->by_default(4)->as_int() - 1;
    led_gcode      = THEKERNEL->config->value( leds_checksum, led_gcode_checksum )->by_default(1)->as_int() - 1;
    led_main       = THEKERNEL->config->value( leds_checksum, led_main_checksum  )->by_default(2)->as_int() - 1;
    led_idle       = THEKERNEL->config->value( leds_checksum, led_idle_checksum  )->by_default(3)->as_int() - 1;
    led_play       = THEKERNEL->config->value( leds_checksum, led_play_checksum  )->by_default(5)->as_int() - 1;
    string mode;
    mode = THEKERNEL->config->value( leds_checksum, led_main_mode_checksum  )->by_default("blink")->as_string();
    if(mode == "blink")
        led_main_mode = MODE_BLINK;
    else
    if(mode == "dimmed")
        led_main_mode = MODE_DIMMED;
    mode = THEKERNEL->config->value( leds_checksum, led_idle_mode_checksum  )->by_default("blink")->as_string();
    if(mode == "blink")
        led_idle_mode = MODE_BLINK;
    else
    if(mode == "dimmed")
        led_idle_mode = MODE_DIMMED;

    counter_gcode = 0;
    counter_main  = 0;
    counter_idle  = 0;
}

void Leds::on_main_init(void* argument)         {
    if(argument) {
        int& post = *(int*)argument;
        for(int bit = 0; bit < n_leds; bit++) {
            leds[bit] = !! (post & (1<<bit));
        }
    }
    else
    if(led_init >= 0)
        leds[led_init] = 1;
}

void Leds::on_sd_ok(void* argument)             {
    if(led_sdok >= 0)
        leds[led_sdok] = 1;
}

void Leds::on_main_loop(void* argument)         {
    if(led_main >= 0) {
        if(led_main_mode == MODE_BLINK)
            leds[led_main]= (counter_main++ & 0x1000) ? 1 : 0;
        else
        if(led_main_mode == MODE_DIMMED)
            leds[led_main]= (counter_main++ & 0x0380) ? 0 : 1;
    }
}

void Leds::on_idle(void* argument)              {
    if(led_idle >= 0) {
        if(led_idle_mode == MODE_BLINK)
            leds[led_idle]= (counter_idle++ & 0x1000) ? 1 : 0;
        else
        if(led_idle_mode == MODE_DIMMED)
            leds[led_idle]= (counter_main++ & 0x0380) ? 0 : 1;
    }
    if(led_gcode >= 0) {
        if(counter_gcode > 0) {
            counter_gcode++;
            if(counter_gcode > 0x0400) {
                counter_gcode = 0;
                leds[led_gcode] = 0;
            }
        }
    }
}

void Leds::on_gcode_received(void* argument)    {
    if(led_gcode >= 0) {
        counter_gcode = 1;
        leds[led_gcode] = 1;
    }
}

uint32_t Leds::half_second_tick(uint32_t)
{
    if(led_play >= 0) {
        if (THEKERNEL->pauser->paused())
            leds[led_play] = ! leds[led_play].get();
        else
            leds[led_play] = ! THEKERNEL->conveyor->is_queue_empty();
    }
    return 0;
}
