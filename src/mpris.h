#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "dbus/dbus_mpris.h"

extern mpris_player players[];
extern int players_count;

int mpris_init();
int mpris_close(void);
int mpris_poll(mpris_player *player);
mpris_player *mpris_get_player_by_namespace(const char *namespace);
int mpris_poll_all();
