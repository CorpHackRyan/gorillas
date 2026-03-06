#include "sound.h"

#include <SDL2/SDL.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef struct Tone {
    float freq_hz;
    int duration_ms;
    float volume;
} Tone;

static SDL_AudioDeviceID g_audio_device = 0;
static SDL_AudioSpec g_audio_spec;
static pid_t g_bgm_pid = -1;
static char g_bgm_temp_path[256] = {0};

extern const unsigned char _binary_brianD_starkiller_mp3_start[];
#ifdef BGM_EMBED_C_ARRAY
extern const size_t _binary_brianD_starkiller_mp3_len;
#else
extern const unsigned char _binary_brianD_starkiller_mp3_end[];
#endif

void sound_stop_bgm(void);

static int command_exists(const char *cmd) {
    const char *path = getenv("PATH");
    const char *start = NULL;
    const char *end = NULL;
    char full[1024];

    if (!path || !cmd || !cmd[0]) {
        return 0;
    }

    start = path;
    while (*start) {
        size_t len = 0;
        end = strchr(start, ':');
        if (!end) {
            end = start + strlen(start);
        }
        len = (size_t)(end - start);
        if (len + 1 + strlen(cmd) + 1 < sizeof(full)) {
            memcpy(full, start, len);
            full[len] = '/';
            strcpy(full + len + 1, cmd);
            if (access(full, X_OK) == 0) {
                return 1;
            }
        }
        start = (*end == ':') ? end + 1 : end;
    }

    return 0;
}

static void queue_tone_sequence(const Tone *tones, size_t tone_count) {
    if (g_audio_device == 0 || g_audio_spec.freq <= 0) {
        return;
    }

    size_t total_samples = 0;
    for (size_t i = 0; i < tone_count; i++) {
        total_samples += (size_t)((tones[i].duration_ms * g_audio_spec.freq) / 1000);
    }

    int16_t *buffer = (int16_t *)malloc(total_samples * sizeof(int16_t));
    if (!buffer) {
        return;
    }

    size_t cursor = 0;
    for (size_t i = 0; i < tone_count; i++) {
        const Tone t = tones[i];
        int sample_count = (t.duration_ms * g_audio_spec.freq) / 1000;
        int ramp = g_audio_spec.freq / 200; /* 5ms fade to reduce clicks */
        if (ramp * 2 > sample_count) {
            ramp = sample_count / 2;
        }

        for (int s = 0; s < sample_count; s++) {
            float amp = t.volume;
            if (ramp > 0) {
                if (s < ramp) {
                    amp *= (float)s / (float)ramp;
                } else if (s > sample_count - ramp) {
                    amp *= (float)(sample_count - s) / (float)ramp;
                }
            }

            float sample = 0.0f;
            if (t.freq_hz > 0.0f) {
                float phase = (2.0f * (float)M_PI * t.freq_hz * (float)s) / (float)g_audio_spec.freq;
                sample = sinf(phase) >= 0.0f ? 1.0f : -1.0f; /* square-wave feel */
            }

            float value = sample * amp * 32767.0f;
            if (value > 32767.0f) {
                value = 32767.0f;
            } else if (value < -32768.0f) {
                value = -32768.0f;
            }
            buffer[cursor++] = (int16_t)value;
        }
    }

    SDL_QueueAudio(g_audio_device, buffer, (Uint32)(cursor * sizeof(int16_t)));
    free(buffer);
}

static int ensure_embedded_bgm_file(void) {
    const unsigned char *start = _binary_brianD_starkiller_mp3_start;
    size_t size = 0;
    int fd = -1;
    ssize_t written = 0;
    size_t offset = 0;
    char path_template[] = "/tmp/gorillas_bgm_XXXXXX.mp3";

    if (g_bgm_temp_path[0] != '\0') {
        return 1;
    }
    if (!start) {
        return 0;
    }
#ifdef BGM_EMBED_C_ARRAY
    size = _binary_brianD_starkiller_mp3_len;
#else
    {
        const unsigned char *end = _binary_brianD_starkiller_mp3_end;
        if (!end || end <= start) {
            return 0;
        }
        size = (size_t)(end - start);
    }
#endif
    fd = mkstemps(path_template, 4);
    if (fd < 0) {
        return 0;
    }

    while (offset < size) {
        written = write(fd, start + offset, size - offset);
        if (written <= 0) {
            close(fd);
            unlink(path_template);
            return 0;
        }
        offset += (size_t)written;
    }
    close(fd);

    strncpy(g_bgm_temp_path, path_template, sizeof(g_bgm_temp_path) - 1);
    g_bgm_temp_path[sizeof(g_bgm_temp_path) - 1] = '\0';
    return 1;
}

int sound_init(void) {
    SDL_AudioSpec want;
    SDL_zero(want);
    want.freq = 44100;
    want.format = AUDIO_S16SYS;
    want.channels = 1;
    want.samples = 2048;

    g_audio_device = SDL_OpenAudioDevice(NULL, 0, &want, &g_audio_spec, 0);
    if (g_audio_device == 0) {
        return 0;
    }

    SDL_PauseAudioDevice(g_audio_device, 0);
    return 1;
}

void sound_shutdown(void) {
    sound_stop_bgm();
    if (g_bgm_temp_path[0] != '\0') {
        unlink(g_bgm_temp_path);
        g_bgm_temp_path[0] = '\0';
    }
    if (g_audio_device != 0) {
        SDL_ClearQueuedAudio(g_audio_device);
        SDL_CloseAudioDevice(g_audio_device);
        g_audio_device = 0;
    }
}

void sound_play_intro(void) {
    /* Approximation of BAS intro jingle. */
    const Tone intro[] = {
        {261.63f, 180, 0.30f}, {293.66f, 180, 0.30f}, {329.63f, 180, 0.30f},
        {293.66f, 180, 0.30f}, {261.63f, 180, 0.30f}, {293.66f, 180, 0.30f},
        {329.63f, 350, 0.30f}, {261.63f, 350, 0.30f}, {261.63f, 350, 0.30f}
    };
    queue_tone_sequence(intro, sizeof(intro) / sizeof(intro[0]));
}

void sound_play_throw(void) {
    /* Approximation of BAS throw chirp. */
    const Tone throw_seq[] = {
        {466.16f, 80, 0.30f}, {523.25f, 80, 0.28f}, {493.88f, 120, 0.28f}, {554.37f, 100, 0.30f}
    };
    queue_tone_sequence(throw_seq, sizeof(throw_seq) / sizeof(throw_seq[0]));
}

void sound_play_explosion(void) {
    const Tone boom[] = {
        {220.00f, 80, 0.35f}, {196.00f, 80, 0.35f}, {164.81f, 100, 0.32f}, {130.81f, 130, 0.30f}
    };
    queue_tone_sequence(boom, sizeof(boom) / sizeof(boom[0]));
}

void sound_play_gorilla_hit(void) {
    /* BAS: PLAY "MBO0L16EFGEFDC" (distinct hit jingle). */
    const Tone hit_seq[] = {
        {164.81f, 125, 0.34f}, /* E */
        {174.61f, 125, 0.34f}, /* F */
        {196.00f, 125, 0.34f}, /* G */
        {164.81f, 125, 0.34f}, /* E */
        {174.61f, 125, 0.34f}, /* F */
        {146.83f, 125, 0.34f}, /* D */
        {130.81f, 125, 0.34f}  /* C */
    };
    queue_tone_sequence(hit_seq, sizeof(hit_seq) / sizeof(hit_seq[0]));
}

int sound_is_bgm_available(void) {
    return command_exists("ffplay");
}

int sound_start_bgm_loop(void) {
    const char *bgm_path = NULL;
    pid_t pid = -1;
    int status = 0;
    char cmd[512];

    if (g_bgm_pid > 0) {
        return 1;
    }
    if (!sound_is_bgm_available()) {
        return 0;
    }

    if (!ensure_embedded_bgm_file()) {
        return 0;
    }
    bgm_path = g_bgm_temp_path;

    pid = fork();
    if (pid < 0) {
        return 0;
    }
    if (pid == 0) {
        setpgid(0, 0);
        snprintf(
            cmd,
            sizeof(cmd),
            "while true; do ffplay -nodisp -autoexit -loglevel quiet '%s' >/dev/null 2>&1; sleep 0.1; done",
            bgm_path
        );
        execl("/bin/sh", "sh", "-c", cmd, (char *)NULL);
        _exit(127);
    }

    g_bgm_pid = pid;
    usleep(100000);
    if (waitpid(g_bgm_pid, &status, WNOHANG) == g_bgm_pid) {
        g_bgm_pid = -1;
        return 0;
    }
    return 1;
}

void sound_stop_bgm(void) {
    if (g_bgm_pid > 0) {
        kill(-g_bgm_pid, SIGTERM);
        waitpid(g_bgm_pid, NULL, 0);
        g_bgm_pid = -1;
    }
}
