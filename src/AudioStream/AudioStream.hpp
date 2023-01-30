// /!\ AudioStream not yet implemented on ESP32, this is a work in progress!!


/* Teensyduino Core Library
* http://www.pjrc.com/teensy/
* Copyright (c) 2017 PJRC.COM, LLC.
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* 1. The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* 2. If the Software is incorporated into a build system that allows
* selection among a list of target devices, then similar target
* devices manufactured by PJRC.COM must be included in the list of
* target devices and selectable in the same manner.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
* BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
* ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
* CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#pragma once
//#ifndef AudioStream_h
#define AudioStream_h

#include <Arduino.h>

#ifndef __ASSEMBLER__
  #include <stdio.h>  // for NULL
  #include <string.h> // for memcpy
#endif


// namespace reSID
// {

  // AUDIO_BLOCK_SAMPLES determines how many samples the audio library processes
  // per update.  It may be reduced to achieve lower latency response to events,
  // at the expense of higher interrupt and DMA setup overhead.
  //
  // Less than 32 may not work with some input & output objects.  Multiples of 16
  // should be used, since some synthesis objects generate 16 samples per loop.
  //
  // Some parts of the audio library may have hard-coded dependency on 128 samples.
  // Please report these on the forum with reproducible test cases.

  #ifndef AUDIO_BLOCK_SAMPLES
    #define AUDIO_BLOCK_SAMPLES 128
  #endif

  #ifndef AUDIO_SAMPLERATE_EXACT
    const float AUDIO_SAMPLERATE_EXACT = 44100.0f;
  #endif

  #define AUDIO_SAMPLE_RATE AUDIO_SAMPLERATE_EXACT

  #ifndef __ASSEMBLER__
  class AudioStream;
  class AudioConnection;

  typedef struct audio_block_struct {
    uint8_t  ref_count;
    uint8_t  reserved1;
    uint16_t memory_pool_index;
    int16_t  data[AUDIO_BLOCK_SAMPLES];
  } audio_block_t;


  class AudioConnection
  {
  public:
    AudioConnection(AudioStream &source, AudioStream &destination) :
      src(source), dst(destination), src_index(0), dest_index(0),
      next_dest(NULL)
      { isConnected = false;
        connect(); }
    AudioConnection(AudioStream &source, unsigned char sourceOutput,
      AudioStream &destination, unsigned char destinationInput) :
      src(source), dst(destination),
      src_index(sourceOutput), dest_index(destinationInput),
      next_dest(NULL)
      { isConnected = false;
        connect(); }
    friend class AudioStream;
    ~AudioConnection() {
      disconnect();
    }
    void disconnect(void);
    void connect(void);
  protected:
    AudioStream &src;
    AudioStream &dst;
    unsigned char src_index;
    unsigned char dest_index;
    AudioConnection *next_dest;
    bool isConnected;
  };


  #define AudioMemory(num) ({ \
    static DMAMEM audio_block_t data[num]; \
    AudioStream::initialize_memory(data, num); \
  })


  // TODO: those are values from teensy core, change them to ESP32 values
  static const uint32_t F_CPU_ACTUAL = 240000000; // esp32 at 240MHz (was: 396000000 for teensy core)
  //static const uint32_t F_BUS_ACTUAL = 132000000;
  static const uint32_t IRQ_SOFTWARE = 94;

  // TODO: find correct IRQ for ESP32
  #if !defined ESP32_CPUINT_TIMER0
  #define ESP32_CPUINT_TIMER0 0
  #endif

  // TODO: turn these into inline asm for ESP32
  // those macros are taken from teensy core, implement ESP32 IRQ !
  __attribute__( ( always_inline ) ) inline void __disable_irq(void)
  {
    portDISABLE_INTERRUPTS();
    //up_disable_irq(ESP32_CPUINT_TIMER0);
    //asm volatile("cpsid i" : : : "memory");
  }
  __attribute__( ( always_inline ) ) inline void __enable_irq(void)
  {
    portENABLE_INTERRUPTS();
    //up_enable_irq(ESP32_CPUINT_TIMER0);
    //asm volatile("cpsie i" : : : "memory");
  }


  #define CYCLE_COUNTER_APPROX_PERCENT(n) (((n) + (F_CPU_ACTUAL / 32 / AUDIO_SAMPLE_RATE * AUDIO_BLOCK_SAMPLES / 100)) / (F_CPU_ACTUAL / 16 / AUDIO_SAMPLE_RATE * AUDIO_BLOCK_SAMPLES / 100))

  #define AudioProcessorUsage() (CYCLE_COUNTER_APPROX_PERCENT(AudioStream::cpu_cycles_total))
  #define AudioProcessorUsageMax() (CYCLE_COUNTER_APPROX_PERCENT(AudioStream::cpu_cycles_total_max))
  #define AudioProcessorUsageMaxReset() (AudioStream::cpu_cycles_total_max = AudioStream::cpu_cycles_total)
  #define AudioMemoryUsage() (AudioStream::memory_used)
  #define AudioMemoryUsageMax() (AudioStream::memory_used_max)
  #define AudioMemoryUsageMaxReset() (AudioStream::memory_used_max = AudioStream::memory_used)

  class AudioStream
  {
  public:
    AudioStream(unsigned char ninput, audio_block_t **iqueue) :
      num_inputs(ninput), inputQueue(iqueue) {
        active = false;
        destination_list = NULL;
        for (int i=0; i < num_inputs; i++) {
          inputQueue[i] = NULL;
        }
        // add to a simple list, for update_all
        // TODO: replace with a proper data flow analysis in update_all
        if (first_update == NULL) {
          first_update = this;
        } else {
          AudioStream *p;
          for (p=first_update; p->next_update; p = p->next_update) ;
          p->next_update = this;
        }
        next_update = NULL;
        cpu_cycles = 0;
        cpu_cycles_max = 0;
        numConnections = 0;
      }
    static void initialize_memory(audio_block_t *data, unsigned int num);
    int processorUsage(void) { return CYCLE_COUNTER_APPROX_PERCENT(cpu_cycles); }
    int processorUsageMax(void) { return CYCLE_COUNTER_APPROX_PERCENT(cpu_cycles_max); }
    void processorUsageMaxReset(void) { cpu_cycles_max = cpu_cycles; }
    bool isActive(void) { return active; }
    uint16_t cpu_cycles;
    uint16_t cpu_cycles_max;
    static uint16_t cpu_cycles_total;
    static uint16_t cpu_cycles_total_max;
    static uint16_t memory_used;
    static uint16_t memory_used_max;
  protected:
    bool active;
    unsigned char num_inputs;
    static audio_block_t * allocate(void);
    static void release(audio_block_t * block);
    void transmit(audio_block_t *block, unsigned char index = 0);
    audio_block_t * receiveReadOnly(unsigned int index = 0);
    audio_block_t * receiveWritable(unsigned int index = 0);
    static bool update_setup(void);
    static void update_stop(void);
    static void update_all(void) { /*NVIC_SET_PENDING(IRQ_SOFTWARE);*/ }
    friend void software_isr(void);
    friend class AudioConnection;
    uint8_t numConnections;
  private:
    AudioConnection *destination_list;
    audio_block_t **inputQueue;
    static bool update_scheduled;
    virtual void update(void) = 0;
    static AudioStream *first_update; // for update_all
    AudioStream *next_update; // for update_all
    static audio_block_t *memory_pool;
    static uint32_t memory_pool_available_mask[];
    static uint16_t memory_pool_first_mask;
  };

  #endif

//};

