#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <lv2.h>

#include "Oscillators.h"

#include "lv2/lv2plug.in/ns/ext/atom/atom.h"
#include "lv2/lv2plug.in/ns/ext/atom/util.h"
#include "lv2/lv2plug.in/ns/ext/midi/midi.h"
#include "lv2/lv2plug.in/ns/ext/urid/urid.h"
#include "lv2/lv2plug.in/ns/lv2core/lv2.h"

/**********************************************************************************************************************************************************/
#define PLUGIN_URI "http://VeJaPlugins.com/plugins/Release/s6lfo"
#define MAX_PORTS 8
#define MAX_OUTPUT_BUFFER_LENGHT 256
#define VCF_LOW_PASS_MODE 0
#define WAVETABLE_SIZE 32768

enum{
    TRIGGER_INPUT,
    OUTPUT,
    LFO_Waveform,
    RATE,
    DELAY
};

using namespace VeJa::Plugins::Oscillators;

class S6_lfo{
public:
    S6_lfo()
    {
        vco = new Oscillator<float>(48000,10, WAVETABLE_SIZE);
    }
    ~S6_lfo() {}
    static LV2_Handle instantiate(const LV2_Descriptor* descriptor, double samplerate, const char* bundle_path, const LV2_Feature* const* features);
    static void activate(LV2_Handle instance);
    static void deactivate(LV2_Handle instance);
    static void connect_port(LV2_Handle instance, uint32_t port, void *data);
    static void run(LV2_Handle instance, uint32_t n_samples);
    static void cleanup(LV2_Handle instance);
    static const void* extension_data(const char* uri);

    float *trigger_input;
    float *output;
    float *waveform;
    float *rate;
    float *delay;

    //important stuff
    float Tau;
    float sampleRate;

    float prev_trigger;

    Oscillator<float> * vco;

};
/**********************************************************************************************************************************************************/

float  MAP(float x, float Omin, float Omax, float Nmin, float Nmax)
{
    return (( x - Omin ) * (Nmax -  Nmin)  / (Omax - Omin) + Nmin);
}      

/**********************************************************************************************************************************************************/
LV2_Handle S6_lfo::instantiate(const LV2_Descriptor*   descriptor,
double                              samplerate,
const char*                         bundle_path,
const LV2_Feature* const* features)
{
    S6_lfo* self = new S6_lfo();

    return (LV2_Handle)self; 
}
/**********************************************************************************************************************************************************/
void S6_lfo::connect_port(LV2_Handle instance, uint32_t port, void *data)
{
    S6_lfo* self = (S6_lfo*)instance;
    switch (port)
    {
        case TRIGGER_INPUT:
            self->trigger_input = (float*) data;
            break;
        case OUTPUT:
            self->output = (float*) data;
            break;
        case LFO_Waveform:
            self->waveform = (float*) data;
            break;
        case RATE:
            self->rate = (float*) data;
            break;
        case DELAY:
            self->delay = (float*) data;
    }
}
/**********************************************************************************************************************************************************/
void S6_lfo::activate(LV2_Handle instance)
{
}

/**********************************************************************************************************************************************************/
void S6_lfo::run(LV2_Handle instance, uint32_t n_samples)
{
    S6_lfo* self = (S6_lfo*)instance;

    float wave = *self->waveform;
    float freq = *self->rate;
    float del = *self->delay;
  	//start audio processing

    for(uint32_t i = 0; i < n_samples; i++)
    {
        float trigger = self->trigger_input[i];

        //delay?
        if (trigger != self->prev_trigger)
        {
            self->prev_trigger = trigger;
                
            if ((trigger == 10) && (del != 0))
            {
                self->vco->SetDelay(del);
            }
        }
        
        //update frequency based on cross mod and vco modulation
        self->vco->NewFrequency(freq);

        //get the audio sample
        self->output[i] = (self->vco->GetDataValue((int)wave) * 10) / 2;
        self->vco->UpdateFrequency();
    }
}   

/**********************************************************************************************************************************************************/
void S6_lfo::deactivate(LV2_Handle instance)
{
    // TODO: include the deactivate function code here
}
/**********************************************************************************************************************************************************/
void S6_lfo::cleanup(LV2_Handle instance)
{
  delete ((S6_lfo *) instance); 
}
/**********************************************************************************************************************************************************/
static const LV2_Descriptor Descriptor = {
    PLUGIN_URI,
    S6_lfo::instantiate,
    S6_lfo::connect_port,
    S6_lfo::activate,
    S6_lfo::run,
    S6_lfo::deactivate,
    S6_lfo::cleanup,
    S6_lfo::extension_data
};
/**********************************************************************************************************************************************************/
LV2_SYMBOL_EXPORT
const LV2_Descriptor* lv2_descriptor(uint32_t index)
{
    if (index == 0) return &Descriptor;
    else return NULL;
}
/**********************************************************************************************************************************************************/
const void* S6_lfo::extension_data(const char* uri)
{
    return NULL;
}
