#include "mpris.h"

mpris_player players[MAX_PLAYERS] = {0};
int players_count = 0;

DBusConnection *conn;

int mpris_init()
{
    // initialise the errors
    DBusError err = {0};
    dbus_error_init(&err);

    // connect to the system bus and check for errors
    conn = dbus_bus_get_private(DBUS_BUS_SESSION, &err);
    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "DBus connection error(%s)\n", err.message);
        dbus_error_free(&err);
    }
    if (conn == NULL)
    {
        printf("Could not init dbus!");
        return 0;
    }

    players_count = load_players(conn, players);

    return 1;
}

int mpris_close()
{
    if (conn != NULL)
    {
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
    }
}

int mpris_poll(mpris_player *player)
{
    int err;

    err = load_mpris_property(conn, player->namespace, MPRIS_PNAME_METADATA, &player->properties);
    if (err) {
        return 0;
    }

    err = load_mpris_property(conn, player->namespace, MPRIS_PNAME_POSITION, &player->properties);
    if (err) {
        return 0;
    }

    err = load_mpris_property(conn, player->namespace, MPRIS_PNAME_PLAYBACKSTATUS, &player->properties);
    if (err) {
        return 0;
    }

    return 1;
}

int mpris_poll_all()
{
    for (int i = 0; i < players_count; i++)
    {
        if (!mpris_poll(&players[i])) {
            return 0;
        }
    }

    return 1;
}

mpris_player *mpris_get_player_by_namespace(const char *namespace)
{
    for (int i = 0; i < players_count; i++)
    {
        if (!strncmp(players[i].namespace, namespace, strlen(namespace)))
        {
            return &players[i];
        }
    }
    return NULL;
}
