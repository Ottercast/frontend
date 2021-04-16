#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "sstring.h"
#include "sdbus.h"

int main(int argc, char** argv)
{

    // initialise the errors
    DBusError err = {0};
    dbus_error_init(&err);

    // connect to the system bus and check for errors
    DBusConnection *conn = dbus_bus_get_private(DBUS_BUS_SESSION, &err);
    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "DBus connection error(%s)\n", err.message);
        dbus_error_free(&err);
    }
    if (NULL == conn) {
        printf("Could not init dbus!");
        return 0;
    }

    mpris_player players[MAX_PLAYERS] = {0};
    int found = load_players(conn, players);
    for (int i = 0; i < found; i++)
    {
        mpris_player player = players[i];
        load_mpris_property(conn, player.namespace, MPRIS_PNAME_METADATA, &player.properties);
        load_mpris_property(conn, player.namespace, MPRIS_PNAME_POSITION, &player.properties);
        load_mpris_property(conn, player.namespace, MPRIS_PNAME_PLAYBACKSTATUS, &player.properties);
    }

    if (NULL != conn) {
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
    }

    return 0;
}

