#include <ESP32-reSID.hpp>
#include <AudioStreamSID/AudioStreamSID.hpp>

#define AUDIO_SAMPLERATE_EXACT 44100.0f
#define MOS_CLOCKFREQ          985248

SID16 *sid16_1, *sid16_2;


void setup()
{
  Serial.begin(115200);
  Serial.println("Hello dual SID");

  sid16_1 = new SID16();
  sid16_2 = new SID16();

  sid16_1->set_chip_model( MOS6581 );
  sid16_2->set_chip_model( MOS6581 );
  sid16_1->reset();
  sid16_2->reset();
  sid16_1->set_sampling_parameters( MOS_CLOCKFREQ, SAMPLE_INTERPOLATE, AUDIO_SAMPLERATE_EXACT );
  sid16_2->set_sampling_parameters( MOS_CLOCKFREQ, SAMPLE_INTERPOLATE, AUDIO_SAMPLERATE_EXACT );
  Serial.println("init done");

}

void loop()
{
}
