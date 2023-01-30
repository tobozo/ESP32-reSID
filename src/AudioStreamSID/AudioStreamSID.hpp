// /!\ AudioStreamSID not yet implemented on ESP32, this is a work in progress!!


/*
   _________.___________   ____  __.__        __
  /   _____/|   \______ \ |    |/ _|__| ____ |  | __
  \_____  \ |   ||    |  \|      < |  |/ ___\|  |/ /
  /        \|   ||    `   \    |  \|  \  \___|    <
 /_______  /|___/_______  /____|__ \__|\___  >__|_ \
         \/             \/        \/       \/     \/

 AudioStreamSID.h

 SIDKick - SID-replacement with SID, Sound Expander and MIDI Emulation based on Teensy 4.1
           (using reSID by Dag Lem and FMOPL by Jarek Burczynski, Tatsuyuki Satoh, Marco van den Heuvel, and Acho A. Tang)
 Copyright (c) 2019-2022 Carsten Dachsbacher <frenetic@dachsbacher.de>

 Logo created with http://patorjk.com/software/taag/

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <Arduino.h>

#include "../ESP32-reSID.hpp"

#include "../AudioStream/AudioStream.hpp"

// namespace reSID
// {



  // TODO: translate this in ESP32 ASM
  //#define DMAMEM __attribute__ ((section(".dmabuffers"), used))
  //#define FASTRUN __attribute__ ((section(".fastrun"), noinline, noclone ))
  #define FASTRUN


  //#include <core_pins.h>


  #define EMULATE_OPL2

  #ifdef EMULATE_OPL2
    #include "../OPL/fmopl.hpp"
  #endif
  //
  // #include "imxrt.h"
  // #include "utility/imxrt_hw.h"
  //
  // #define CB( name, i, n ) { if ( (int)(i) < 0 || (int)(i) >= (int)(n) ) {Serial.print( name ); Serial.print( " out of bounds: " ); Serial.println( (int)i );} };

  extern unsigned int CLOCKFREQ, CLOCKFREQ_NOMINAL;
  extern void speakSAM( uint8_t digit, bool onlyDigit = false );

  class AudioStreamSID : public AudioStream
  {
  public:
    AudioStreamSID(void) : AudioStream( 0, NULL) {}
    void init();
    void begin();
    void reset();
    void stop();
    void continuePlaying();
    void fillBlock();
    void quickreset();
    void updateConfiguration( uint8_t *cfg, uint8_t *globalCfg );

  private:
    volatile bool playing;
    virtual void update();
    SID16 *sid16_1, *sid16_2;
  #ifndef NO_RESID10
    RESID_NAMESPACE::SID *sid_1, *sid_2;
  #endif

    #ifdef EMULATE_OPL2
    FM_OPL *pOPL;
    uint32_t fmOutRegister, fmFakeOutput;
    uint8_t hack_OPL_Sample_Value;
    uint8_t hack_OPL_Sample_Enabled;
    #endif

  public:
  #ifndef NO_RESID10
    RESID_NAMESPACE::SID *getSID() { return sid_1; };
  #endif
    SID16 *getSID16() { return sid16_1; };

    void updateMixer( bool playSID2, bool playFM );
  };


//};
