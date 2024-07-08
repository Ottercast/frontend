#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include <dbus/dbus.h>

#define MAX_OUTPUT_LENGTH 1024
#define LOCAL_NAME "org.mpris.mprisctl"
#define MPRIS_PLAYER_NAMESPACE "org.mpris.MediaPlayer2"
#define MPRIS_PLAYER_PATH "/org/mpris/MediaPlayer2"
#define MPRIS_PLAYER_INTERFACE "org.mpris.MediaPlayer2.Player"
#define MPRIS_METHOD_NEXT "Next"
#define MPRIS_METHOD_PREVIOUS "Previous"
#define MPRIS_METHOD_PLAY "Play"
#define MPRIS_METHOD_PAUSE "Pause"
#define MPRIS_METHOD_STOP "Stop"
#define MPRIS_METHOD_PLAY_PAUSE "PlayPause"

#define MPRIS_PNAME_PLAYBACKSTATUS "PlaybackStatus"
#define MPRIS_PNAME_CANCONTROL "CanControl"
#define MPRIS_PNAME_CANGONEXT "CanGoNext"
#define MPRIS_PNAME_CANGOPREVIOUS "CanGoPrevious"
#define MPRIS_PNAME_CANPLAY "CanPlay"
#define MPRIS_PNAME_CANPAUSE "CanPause"
#define MPRIS_PNAME_CANSEEK "CanSeek"
#define MPRIS_PNAME_SHUFFLE "Shuffle"
#define MPRIS_PNAME_POSITION "Position"
#define MPRIS_PNAME_VOLUME "Volume"
#define MPRIS_PNAME_LOOPSTATUS "LoopStatus"
#define MPRIS_PNAME_METADATA "Metadata"

#define MPRIS_PROP_PLAYBACK_STATUS "PlaybackStatus"
#define MPRIS_PROP_METADATA "Metadata"
#define MPRIS_ARG_PLAYER_IDENTITY "Identity"

#define DBUS_DESTINATION "org.freedesktop.DBus"
#define DBUS_PATH "/"
#define DBUS_INTERFACE "org.freedesktop.DBus"
#define DBUS_PROPERTIES_INTERFACE "org.freedesktop.DBus.Properties"
#define DBUS_METHOD_LIST_NAMES "ListNames"
#define DBUS_METHOD_GET_ALL "GetAll"
#define DBUS_METHOD_GET "Get"

#define MPRIS_METADATA_BITRATE "bitrate"
#define MPRIS_METADATA_ART_URL "mpris:artUrl"
#define MPRIS_METADATA_LENGTH "mpris:length"
#define MPRIS_METADATA_TRACKID "mpris:trackid"
#define MPRIS_METADATA_ALBUM "xesam:album"
#define MPRIS_METADATA_ALBUM_ARTIST "xesam:albumArtist"
#define MPRIS_METADATA_ARTIST "xesam:artist"
#define MPRIS_METADATA_COMMENT "xesam:comment"
#define MPRIS_METADATA_TITLE "xesam:title"
#define MPRIS_METADATA_TRACK_NUMBER "xesam:trackNumber"
#define MPRIS_METADATA_URL "xesam:url"
#define MPRIS_METADATA_YEAR "year"

#define MPRIS_METADATA_VALUE_STOPPED "Stopped"
#define MPRIS_METADATA_VALUE_PLAYING "Playing"
#define MPRIS_METADATA_VALUE_PAUSED "Paused"

// The default timeout leads to hangs when calling
//   certain players which don't seem to reply to MPRIS methods
#define DBUS_CONNECTION_TIMEOUT 1000 //ms

#define MAX_PLAYERS 20

typedef struct mpris_metadata
{
    uint64_t length; // mpris specific
    unsigned short track_number;
    unsigned short bitrate;
    unsigned short disc_number;
    char album_artist[MAX_OUTPUT_LENGTH];
    char composer[MAX_OUTPUT_LENGTH];
    char genre[MAX_OUTPUT_LENGTH];
    char artist[MAX_OUTPUT_LENGTH];
    char comment[MAX_OUTPUT_LENGTH];
    char track_id[MAX_OUTPUT_LENGTH];
    char album[MAX_OUTPUT_LENGTH];
    char content_created[MAX_OUTPUT_LENGTH];
    char title[MAX_OUTPUT_LENGTH];
    char url[MAX_OUTPUT_LENGTH];
    char art_url[MAX_OUTPUT_LENGTH]; //mpris specific

} mpris_metadata;

typedef struct mpris_properties
{
    double volume;
    uint64_t position;
    bool can_control;
    bool can_go_next;
    bool can_go_previous;
    bool can_play;
    bool can_pause;
    bool can_seek;
    bool shuffle;
    char player_name[MAX_OUTPUT_LENGTH];
    char loop_status[MAX_OUTPUT_LENGTH];
    char playback_status[MAX_OUTPUT_LENGTH];
    mpris_metadata metadata;
} mpris_properties;

typedef struct mpris_player
{
    char *name;
    char namespace[MAX_OUTPUT_LENGTH];
    mpris_properties properties;
    bool active;
} mpris_player;

DBusMessage *call_dbus_method(DBusConnection *conn, char *destination, char *path, char *interface, char *method);
int load_mpris_property(DBusConnection *conn, const char *destination, const char *arg_identity, mpris_properties *properties);
int load_players(DBusConnection *conn, mpris_player *players);
