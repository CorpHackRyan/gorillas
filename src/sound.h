#ifndef SOUND_H
#define SOUND_H

int sound_init(void);
void sound_shutdown(void);

void sound_play_intro(void);
void sound_play_throw(void);
void sound_play_explosion(void);
void sound_play_gorilla_hit(void);
int sound_is_bgm_available(void);
int sound_start_bgm_loop(void);
void sound_stop_bgm(void);

#endif
