#include "libretro.h"

#include <cstdarg>
#include <cstring>
#include <cstdio>

#include <array>

static void fallback_log(enum retro_log_level level, const char* fmt, ...)
{
  (void)level;
  va_list va;
  va_start(va, fmt);
  vfprintf(stderr, fmt, va);
  va_end(va);
}

struct RetroArchEnv
{
  retro_video_refresh_t video;
  retro_audio_sample_t audio;
  retro_audio_sample_batch_t audioBatch;
  retro_input_poll_t inputPoll;
  retro_input_state_t inputState;
  retro_log_printf_t logger = fallback_log;

  uint32_t frameCounter;
  uint16_t buttonState;
};

RetroArchEnv env;

extern "C"
{
  unsigned retro_api_version()
  {
    return RETRO_API_VERSION;
  }

  void retro_init()
  {

  }

  void retro_get_system_info(struct retro_system_info* info)
  {
    info->library_name = "Abab Is Me";
    info->library_version = "0.1";
    info->need_fullpath = false;
    info->valid_extensions = "zip";
    info->block_extract = false;
  }

  void retro_get_system_av_info(struct retro_system_av_info* info)
  {
    info->geometry.base_height = 480;
    info->geometry.base_width = 480;
    info->geometry.max_height = 480;
    info->geometry.max_width = 480;
    info->geometry.aspect_ratio = 1.0f;

    info->timing.fps = 60.0f;
    info->timing.sample_rate = 0;
  }

  void retro_set_environment(retro_environment_t e)
  {
    retro_pixel_format pixelFormat = RETRO_PIXEL_FORMAT_XRGB8888;
    e(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &pixelFormat);

    retro_log_callback logger;
    if (e(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &logger))
      env.logger = logger.log;
  }

  void retro_set_video_refresh(retro_video_refresh_t callback) { env.video = callback; }
  void retro_set_audio_sample(retro_audio_sample_t callback) { env.audio = callback; }
  void retro_set_audio_sample_batch(retro_audio_sample_batch_t callback) { env.audioBatch = callback; }
  void retro_set_input_poll(retro_input_poll_t callback) { env.inputPoll = callback; }
  void retro_set_input_state(retro_input_state_t callback) { env.inputState = callback; }
  void retro_set_controller_port_device(unsigned port, unsigned device) { /* TODO */ }

  size_t retro_serialize_size(void) { return 0; }
  bool retro_serialize(void* data, size_t size) { return true; }
  bool retro_unserialize(const void* data, size_t size) { return true; }
  void retro_cheat_reset(void) { }
  void retro_cheat_set(unsigned index, bool enabled, const char* code) { }
  unsigned retro_get_region(void) { return 0; }
  void* retro_get_memory_data(unsigned id) { return nullptr; }
  size_t retro_get_memory_size(unsigned id) { return 0; }

  bool retro_load_game(const struct retro_game_info* game)
  {
    return true;
  }

  void retro_run()
  {
    //env.video()
    ++env.frameCounter;

    //env.audioBatch(nullptr, 0);

    /* manage input */
    {
      struct BtPair {
        unsigned player;
        int16_t rabt;
        size_t r8bt;
        bool isSet;
      };

      static std::array<BtPair, 8> mapping = { {
        { 0, RETRO_DEVICE_ID_JOYPAD_LEFT, 0, false },
        { 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, 1, false },
        { 0, RETRO_DEVICE_ID_JOYPAD_UP, 2, false },
        { 0, RETRO_DEVICE_ID_JOYPAD_DOWN, 3, false },
        { 0, RETRO_DEVICE_ID_JOYPAD_A, 4, false },
        { 0, RETRO_DEVICE_ID_JOYPAD_B, 5, false },
        { 1, RETRO_DEVICE_ID_JOYPAD_X, 4, false },
        { 1, RETRO_DEVICE_ID_JOYPAD_Y, 5, false },

      } };

      env.inputPoll();
      for (auto& entry : mapping)
      {
        const bool isSet = env.inputState(entry.player, RETRO_DEVICE_JOYPAD, 0, entry.rabt);
        const bool wasSet = entry.isSet;

        //if (wasSet != isSet)
        //  input.manageKey(entry.player, entry.r8bt, isSet);

        entry.isSet = isSet;
      }
    }
  }

  void retro_reset()
  {

  }   
}

