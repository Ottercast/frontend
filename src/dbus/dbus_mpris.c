/**
 * DBUS/MPRIS implementation is based on https://github.com/mariusor/mpris-ctl
 * Copyright (c) Marius Orcsik <marius@habarnam.ro>
 */

#include "dbus_mpris.h"

#include <errno.h>

void mpris_metadata_init(mpris_metadata* metadata)
{
    metadata->track_number = 0;
    metadata->bitrate = 0;
    metadata->disc_number = 0;
    metadata->length = 0;
    memcpy(metadata->album_artist, "unknown", 8);
    memcpy(metadata->composer, "unknown", 8);
    memcpy(metadata->genre, "unknown", 8);
    memcpy(metadata->artist, "unknown", 8);
    memcpy(metadata->album, "unknown", 8);
    memcpy(metadata->title, "unknown", 8);
}

DBusMessage* call_dbus_method(DBusConnection* conn, char* destination, char* path, char* interface, char* method)
{
    if (NULL == conn) { return NULL; }
    if (NULL == destination) { return NULL; }

    DBusMessage* msg;
    DBusPendingCall* pending;

    // create a new method call and check for errors
    msg = dbus_message_new_method_call(destination, path, interface, method);
    if (NULL == msg) { return NULL; }

    // send message and get a handle for a reply
    if (!dbus_connection_send_with_reply (conn, msg, &pending, DBUS_CONNECTION_TIMEOUT)) {
        goto _unref_message_err;
    }
    if (NULL == pending) {
        goto _unref_message_err;
    }
    dbus_connection_flush(conn);

    // free message
    dbus_message_unref(msg);

    // block until we receive a reply
    dbus_pending_call_block(pending);

    DBusMessage* reply;
    // get the reply message
    reply = dbus_pending_call_steal_reply(pending);

    // free the pending message handle
    dbus_pending_call_unref(pending);

    return reply;

_unref_message_err:
    {
        dbus_message_unref(msg);
    }
    return NULL;
}

double extract_double_var(DBusMessageIter *iter, DBusError *error)
{
    double result = 0;

    if (DBUS_TYPE_VARIANT != dbus_message_iter_get_arg_type(iter)) {
        dbus_set_error_const(error, "iter_should_be_variant", "This message iterator must be have variant type");
        return 0;
    }

    DBusMessageIter variantIter;
    dbus_message_iter_recurse(iter, &variantIter);
    if (DBUS_TYPE_DOUBLE == dbus_message_iter_get_arg_type(&variantIter)) {
        dbus_message_iter_get_basic(&variantIter, &result);
        return result;
    }
    return 0;
}

void extract_string_var(char result[MAX_OUTPUT_LENGTH], DBusMessageIter *iter, DBusError *error)
{
    if (DBUS_TYPE_VARIANT != dbus_message_iter_get_arg_type(iter)) {
        dbus_set_error_const(error, "iter_should_be_variant", "This message iterator must be have variant type");
        return;
    }
    memset(result, 0, MAX_OUTPUT_LENGTH);

    DBusMessageIter variantIter = {0};
    dbus_message_iter_recurse(iter, &variantIter);
    if (DBUS_TYPE_OBJECT_PATH == dbus_message_iter_get_arg_type(&variantIter)) {
        char *val = NULL;
        dbus_message_iter_get_basic(&variantIter, &val);
        memcpy(result, val, strlen(val));
    } else if (DBUS_TYPE_STRING == dbus_message_iter_get_arg_type(&variantIter)) {
        char *val = NULL;
        dbus_message_iter_get_basic(&variantIter, &val);
        memcpy(result, val, strlen(val));
    } else if (DBUS_TYPE_ARRAY == dbus_message_iter_get_arg_type(&variantIter)) {
        DBusMessageIter arrayIter;
        dbus_message_iter_recurse(&variantIter, &arrayIter);
        while (true) {
            // todo(marius): load all elements of the array
            if (DBUS_TYPE_STRING == dbus_message_iter_get_arg_type(&arrayIter)) {
                char *val = NULL;
                dbus_message_iter_get_basic(&arrayIter, &val);
                strncat(result, val, MAX_OUTPUT_LENGTH - strlen(result) - 1);
            }
            if (!dbus_message_iter_has_next(&arrayIter)) {
                break;
            }
            dbus_message_iter_next(&arrayIter);
        }
    }
}

int32_t extract_int32_var(DBusMessageIter *iter, DBusError *error)
{
    int32_t result = 0;
    if (DBUS_TYPE_VARIANT != dbus_message_iter_get_arg_type(iter)) {
        dbus_set_error_const(error, "iter_should_be_variant", "This message iterator must be have variant type");
        return 0;
    }

    DBusMessageIter variantIter;
    dbus_message_iter_recurse(iter, &variantIter);

    if (DBUS_TYPE_INT32 == dbus_message_iter_get_arg_type(&variantIter)) {
        dbus_message_iter_get_basic(&variantIter, &result);
        return result;
    }
    return 0;
}

int64_t extract_int64_var(DBusMessageIter *iter, DBusError *error)
{
    int64_t result = 0;
    if (DBUS_TYPE_VARIANT != dbus_message_iter_get_arg_type(iter)) {
        dbus_set_error_const(error, "iter_should_be_variant", "This message iterator must be have variant type");
        return 0;
    }

    DBusMessageIter variantIter;
    dbus_message_iter_recurse(iter, &variantIter);

    if (DBUS_TYPE_INT64 == dbus_message_iter_get_arg_type(&variantIter)) {
        dbus_message_iter_get_basic(&variantIter, &result);
        return result;
    }
    if (DBUS_TYPE_UINT64 == dbus_message_iter_get_arg_type(&variantIter)) {
        uint64_t temp;
        dbus_message_iter_get_basic(&variantIter, &temp);
        result = (int64_t)temp;
        return result;
    }
    return 0;
}

bool extract_boolean_var(DBusMessageIter *iter, DBusError *error)
{
    bool *result = false;

    if (DBUS_TYPE_VARIANT != dbus_message_iter_get_arg_type(iter)) {
        dbus_set_error_const(error, "iter_should_be_variant", "This message iterator must be have variant type");
        return false;
    }

    DBusMessageIter variantIter;
    dbus_message_iter_recurse(iter, &variantIter);

    if (DBUS_TYPE_BOOLEAN == dbus_message_iter_get_arg_type(&variantIter)) {
        dbus_message_iter_get_basic(&variantIter, &result);
        return result;
    }
    return false;
}

void load_metadata(mpris_metadata *track, DBusMessageIter *iter)
{
    DBusError err;
    dbus_error_init(&err);

    if (DBUS_TYPE_VARIANT != dbus_message_iter_get_arg_type(iter)) {
        dbus_set_error_const(&err, "iter_should_be_variant", "This message iterator must be have variant type");
        return;
    }

    DBusMessageIter variantIter;
    dbus_message_iter_recurse(iter, &variantIter);
    if (DBUS_TYPE_ARRAY != dbus_message_iter_get_arg_type(&variantIter)) {
        dbus_set_error_const(&err, "variant_should_be_array", "This variant reply message must have array content");
        return;
    }
    DBusMessageIter arrayIter;
    dbus_message_iter_recurse(&variantIter, &arrayIter);
    while (true) {
        char* key = NULL;
        if (DBUS_TYPE_DICT_ENTRY == dbus_message_iter_get_arg_type(&arrayIter)) {
            DBusMessageIter dictIter;
            dbus_message_iter_recurse(&arrayIter, &dictIter);
            if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&dictIter)) {
                dbus_set_error_const(&err, "missing_key", "This message iterator doesn't have key");
            }
            dbus_message_iter_get_basic(&dictIter, &key);

            if (!dbus_message_iter_has_next(&dictIter)) {
                continue;
            }
            dbus_message_iter_next(&dictIter);

            if (!strncmp(key, MPRIS_METADATA_BITRATE, strlen(MPRIS_METADATA_BITRATE))) {
                track->bitrate = extract_int32_var(&dictIter, &err);
            }
            if (!strncmp(key, MPRIS_METADATA_ART_URL, strlen(MPRIS_METADATA_ART_URL))) {
                extract_string_var(track->art_url, &dictIter, &err);
            }
            if (!strncmp(key, MPRIS_METADATA_LENGTH, strlen(MPRIS_METADATA_LENGTH))) {
                track->length = extract_int64_var(&dictIter, &err);
            }
            if (!strncmp(key, MPRIS_METADATA_TRACKID, strlen(MPRIS_METADATA_TRACKID))) {
                extract_string_var(track->track_id, &dictIter, &err);
            }
            if (!strncmp(key, MPRIS_METADATA_ALBUM_ARTIST, strlen(MPRIS_METADATA_ALBUM_ARTIST))) {
                extract_string_var(track->album_artist, &dictIter, &err);
            } else if (!strncmp(key, MPRIS_METADATA_ALBUM, strlen(MPRIS_METADATA_ALBUM))) {
                extract_string_var(track->album, &dictIter, &err);
            }
            if (!strncmp(key, MPRIS_METADATA_ARTIST, strlen(MPRIS_METADATA_ARTIST))) {
                extract_string_var(track->artist, &dictIter, &err);
            }
            if (!strncmp(key, MPRIS_METADATA_COMMENT, strlen(MPRIS_METADATA_COMMENT))) {
                extract_string_var(track->comment, &dictIter, &err);
            }
            if (!strncmp(key, MPRIS_METADATA_TITLE, strlen(MPRIS_METADATA_TITLE))) {
                extract_string_var(track->title, &dictIter, &err);
            }
            if (!strncmp(key, MPRIS_METADATA_TRACK_NUMBER, strlen(MPRIS_METADATA_TRACK_NUMBER))) {
                track->track_number = extract_int32_var(&dictIter, &err);
            }
            if (!strncmp(key, MPRIS_METADATA_URL, strlen(MPRIS_METADATA_URL))) {
                extract_string_var(track->url, &dictIter, &err);
            }
            if (dbus_error_is_set(&err)) {
                fprintf(stderr, "err: %s, %s\n", key, err.message);
                dbus_error_free(&err);
            }
        }
        if (!dbus_message_iter_has_next(&arrayIter)) {
            break;
        }
        dbus_message_iter_next(&arrayIter);
    }
}

void get_player_identity(char *identity, DBusConnection *conn, const char* destination)
{
    if (NULL == conn) { return; }
    if (NULL == destination) { return; }
    if (NULL == identity) { return; }
    if (strncmp(MPRIS_PLAYER_NAMESPACE, destination, strlen(MPRIS_PLAYER_NAMESPACE))) { return; }

    DBusMessage* msg;
    DBusError err;
    DBusPendingCall* pending;
    DBusMessageIter params;

    char *interface = DBUS_PROPERTIES_INTERFACE;
    char *method = DBUS_METHOD_GET;
    char *path = MPRIS_PLAYER_PATH;
    char *arg_interface = MPRIS_PLAYER_NAMESPACE;
    char *arg_identity = MPRIS_ARG_PLAYER_IDENTITY;

    dbus_error_init(&err);
    // create a new method call and check for errors
    msg = dbus_message_new_method_call(destination, path, interface, method);
    if (NULL == msg) { return; }

    // append interface we want to get the property from
    dbus_message_iter_init_append(msg, &params);
    if (!dbus_message_iter_append_basic(&params, DBUS_TYPE_STRING, &arg_interface)) {
        goto _unref_message_err;
    }

    dbus_message_iter_init_append(msg, &params);
    if (!dbus_message_iter_append_basic(&params, DBUS_TYPE_STRING, &arg_identity)) {
        goto _unref_message_err;
    }

    // send message and get a handle for a reply
    if (!dbus_connection_send_with_reply (conn, msg, &pending, DBUS_CONNECTION_TIMEOUT)) {
        goto _unref_message_err;
    }
    if (NULL == pending) {
        goto _unref_message_err;
    }
    dbus_connection_flush(conn);

    // block until we receive a reply
    dbus_pending_call_block(pending);

    DBusMessage* reply;
    // get the reply message
    reply = dbus_pending_call_steal_reply(pending);
    if (NULL == reply) { goto _unref_pending_err; }

    DBusMessageIter rootIter;
    if (dbus_message_iter_init(reply, &rootIter)) {
        extract_string_var(identity, &rootIter, &err);
    }
    if (dbus_error_is_set(&err)) {
        dbus_error_free(&err);
    }

    dbus_message_unref(reply);
    // free the pending message handle
_unref_pending_err:
    dbus_pending_call_unref(pending);
_unref_message_err:
    // free message
    dbus_message_unref(msg);
}

int load_mpris_property(DBusConnection *conn, const char *destination, const char *arg_identity, mpris_properties *properties)
{
    if (NULL == conn)
    {
        return -ENOENT;
    }
    if (NULL == destination)
    {
        return -EINVAL;
    }

    DBusMessage *msg;
    DBusPendingCall *pending;
    DBusMessageIter params;
    DBusError dbus_err;

    dbus_error_init(&dbus_err);

    char *interface = DBUS_PROPERTIES_INTERFACE;
    char *method = DBUS_METHOD_GET;
    char *path = MPRIS_PLAYER_PATH;
    char *arg_interface = MPRIS_PLAYER_INTERFACE;

    // create a new method call and check for errors
    msg = dbus_message_new_method_call(destination, path, interface, method);
    if (NULL == msg)
    {
        return -ENOMEM;
    }

    int err = 0;
    // append interface we want to get the property from
    dbus_message_iter_init_append(msg, &params);
    if (!dbus_message_iter_append_basic(&params, DBUS_TYPE_STRING, &arg_interface))
    {
        err = -EIO;
        goto _unref_message_err;
    }

    dbus_message_iter_init_append(msg, &params);
    if (!dbus_message_iter_append_basic(&params, DBUS_TYPE_STRING, &arg_identity))
    {
        err = -EIO;
        goto _unref_message_err;
    }

    // send message and get a handle for a reply
    if (!dbus_connection_send_with_reply(conn, msg, &pending, DBUS_CONNECTION_TIMEOUT))
    {
        err = -EIO;
        goto _unref_message_err;
    }
    if (NULL == pending)
    {
        err = -ENOMEM;
        goto _unref_message_err;
    }
    dbus_connection_flush(conn);
    // block until we receive a reply
    dbus_pending_call_block(pending);

    DBusMessage *reply;
    // get the reply message
    reply = dbus_pending_call_steal_reply(pending);
    if (NULL == reply)
    {
        printf("empty reply error!\n");
        err = -EIO;
        goto _unref_pending_err;
    }
    DBusMessageIter rootIter;

    dbus_message_iter_init(reply, &rootIter);

    if (!strncmp(arg_identity, MPRIS_PNAME_CANCONTROL, strlen(MPRIS_PNAME_CANCONTROL)))
    {
        properties->can_control = extract_boolean_var(&rootIter, &dbus_err);
    }
    if (!strncmp(arg_identity, MPRIS_PNAME_CANGONEXT, strlen(MPRIS_PNAME_CANGONEXT)))
    {
        properties->can_go_next = extract_boolean_var(&rootIter, &dbus_err);
    }
    if (!strncmp(arg_identity, MPRIS_PNAME_CANGOPREVIOUS, strlen(MPRIS_PNAME_CANGOPREVIOUS)))
    {
        properties->can_go_previous = extract_boolean_var(&rootIter, &dbus_err);
    }
    if (!strncmp(arg_identity, MPRIS_PNAME_CANPAUSE, strlen(MPRIS_PNAME_CANPAUSE)))
    {
        properties->can_pause = extract_boolean_var(&rootIter, &dbus_err);
    }
    if (!strncmp(arg_identity, MPRIS_PNAME_CANPLAY, strlen(MPRIS_PNAME_CANPLAY)))
    {
        properties->can_play = extract_boolean_var(&rootIter, &dbus_err);
    }
    if (!strncmp(arg_identity, MPRIS_PNAME_CANSEEK, strlen(MPRIS_PNAME_CANSEEK)))
    {
        properties->can_seek = extract_boolean_var(&rootIter, &dbus_err);
    }
    if (!strncmp(arg_identity, MPRIS_PNAME_LOOPSTATUS, strlen(MPRIS_PNAME_LOOPSTATUS)))
    {
        extract_string_var(properties->loop_status, &rootIter, &dbus_err);
    }
    if (!strncmp(arg_identity, MPRIS_PNAME_METADATA, strlen(MPRIS_PNAME_METADATA)))
    {
        load_metadata(&properties->metadata, &rootIter);
    }
    if (!strncmp(arg_identity, MPRIS_PNAME_PLAYBACKSTATUS, strlen(MPRIS_PNAME_PLAYBACKSTATUS)))
    {
        extract_string_var(properties->playback_status, &rootIter, &dbus_err);
    }
    if (!strncmp(arg_identity, MPRIS_PNAME_POSITION, strlen(MPRIS_PNAME_POSITION)))
    {
        properties->position = extract_int64_var(&rootIter, &dbus_err);
    }
    if (!strncmp(arg_identity, MPRIS_PNAME_SHUFFLE, strlen(MPRIS_PNAME_SHUFFLE)))
    {
        properties->shuffle = extract_boolean_var(&rootIter, &dbus_err);
    }
    if (!strncmp(arg_identity, MPRIS_PNAME_VOLUME, strlen(MPRIS_PNAME_VOLUME)))
    {
        properties->volume = extract_double_var(&rootIter, &dbus_err);
    }
    if (dbus_error_is_set(&dbus_err))
    {
        fprintf(stderr, "error: %s arg: %s\n", dbus_err.message, arg_identity);
        err = -EIO;
        dbus_error_free(&dbus_err);
    }

    dbus_message_unref(reply);
    // free the pending message handle
    dbus_pending_call_unref(pending);
    // free message
    dbus_message_unref(msg);

    get_player_identity(properties->player_name, conn, destination);
    return err;

_unref_pending_err:
    dbus_pending_call_unref(pending);
_unref_message_err:
    dbus_message_unref(msg);
    return err;
}

void load_mpris_properties(DBusConnection * conn, const char *destination, mpris_properties *properties)
{
    if (NULL == conn)
    {
        return;
    }
    if (NULL == destination)
    {
        return;
    }

    DBusMessage *msg;
    DBusPendingCall *pending;
    DBusMessageIter params;
    DBusError err;

    dbus_error_init(&err);

    char *interface = DBUS_PROPERTIES_INTERFACE;
    char *method = DBUS_METHOD_GET;
    char *path = MPRIS_PLAYER_PATH;
    char *arg_interface = MPRIS_PLAYER_INTERFACE;
    char *arg_identity = "Metadata";

    //printf("%s %s %s %s %s\n", interface, method, path, arg_interface, destination);

    // create a new method call and check for errors
    msg = dbus_message_new_method_call(destination, path, interface, method);
    if (NULL == msg)
    {
        return;
    }

    // append interface we want to get the property from
    dbus_message_iter_init_append(msg, &params);
    if (!dbus_message_iter_append_basic(&params, DBUS_TYPE_STRING, &arg_interface))
    {
        goto _unref_message_err;
    }

    dbus_message_iter_init_append(msg, &params);
    if (!dbus_message_iter_append_basic(&params, DBUS_TYPE_STRING, &arg_identity))
    {
        goto _unref_message_err;
    }

    // send message and get a handle for a reply
    if (!dbus_connection_send_with_reply(conn, msg, &pending, DBUS_CONNECTION_TIMEOUT))
    {
        goto _unref_message_err;
    }
    if (NULL == pending)
    {
        goto _unref_message_err;
    }
    dbus_connection_flush(conn);
    // block until we receive a reply
    dbus_pending_call_block(pending);

    DBusMessage *reply;
    // get the reply message
    reply = dbus_pending_call_steal_reply(pending);
    if (NULL == reply)
    {
        goto _unref_pending_err;
    }
    DBusMessageIter rootIter;

    dbus_message_iter_init(reply, &rootIter);
    printf("arg type: %c\n", dbus_message_iter_get_arg_type(&rootIter));

    load_metadata(&properties->metadata, &rootIter);

    printf("title: %s\n", (&properties->metadata)->title);

    if (dbus_message_iter_init(reply, &rootIter) && DBUS_TYPE_ARRAY == dbus_message_iter_get_arg_type(&rootIter))
    {
        DBusMessageIter arrayElementIter;

        dbus_message_iter_recurse(&rootIter, &arrayElementIter);
        while (true)
        {
            char *key;
            if (DBUS_TYPE_DICT_ENTRY == dbus_message_iter_get_arg_type(&arrayElementIter))
            {
                DBusMessageIter dictIter;
                dbus_message_iter_recurse(&arrayElementIter, &dictIter);
                if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&dictIter))
                {
                    dbus_set_error_const(&err, "missing_key", "This message iterator doesn't have key");
                }
                dbus_message_iter_get_basic(&dictIter, &key);

                if (!dbus_message_iter_has_next(&dictIter))
                {
                    continue;
                }
                dbus_message_iter_next(&dictIter);

                if (!strncmp(key, MPRIS_PNAME_CANCONTROL, strlen(MPRIS_PNAME_CANCONTROL)))
                {
                    properties->can_control = extract_boolean_var(&dictIter, &err);
                }
                if (!strncmp(key, MPRIS_PNAME_CANGONEXT, strlen(MPRIS_PNAME_CANGONEXT)))
                {
                    properties->can_go_next = extract_boolean_var(&dictIter, &err);
                }
                if (!strncmp(key, MPRIS_PNAME_CANGOPREVIOUS, strlen(MPRIS_PNAME_CANGOPREVIOUS)))
                {
                    properties->can_go_previous = extract_boolean_var(&dictIter, &err);
                }
                if (!strncmp(key, MPRIS_PNAME_CANPAUSE, strlen(MPRIS_PNAME_CANPAUSE)))
                {
                    properties->can_pause = extract_boolean_var(&dictIter, &err);
                }
                if (!strncmp(key, MPRIS_PNAME_CANPLAY, strlen(MPRIS_PNAME_CANPLAY)))
                {
                    properties->can_play = extract_boolean_var(&dictIter, &err);
                }
                if (!strncmp(key, MPRIS_PNAME_CANSEEK, strlen(MPRIS_PNAME_CANSEEK)))
                {
                    properties->can_seek = extract_boolean_var(&dictIter, &err);
                }
                if (!strncmp(key, MPRIS_PNAME_LOOPSTATUS, strlen(MPRIS_PNAME_LOOPSTATUS)))
                {
                    extract_string_var(properties->loop_status, &dictIter, &err);
                }
                if (!strncmp(key, MPRIS_PNAME_METADATA, strlen(MPRIS_PNAME_METADATA)))
                {
                    load_metadata(&properties->metadata, &dictIter);
                }
                if (!strncmp(key, MPRIS_PNAME_PLAYBACKSTATUS, strlen(MPRIS_PNAME_PLAYBACKSTATUS)))
                {
                    extract_string_var(properties->playback_status, &dictIter, &err);
                }
                if (!strncmp(key, MPRIS_PNAME_POSITION, strlen(MPRIS_PNAME_POSITION)))
                {
                    properties->position = extract_int64_var(&dictIter, &err);
                }
                if (!strncmp(key, MPRIS_PNAME_SHUFFLE, strlen(MPRIS_PNAME_SHUFFLE)))
                {
                    properties->shuffle = extract_boolean_var(&dictIter, &err);
                }
                if (!strncmp(key, MPRIS_PNAME_VOLUME, strlen(MPRIS_PNAME_VOLUME)))
                {
                    properties->volume = extract_double_var(&dictIter, &err);
                }
                if (dbus_error_is_set(&err))
                {
                    fprintf(stderr, "error: %s\n", err.message);
                    dbus_error_free(&err);
                }
            }
            if (!dbus_message_iter_has_next(&arrayElementIter))
            {
                break;
            }
            dbus_message_iter_next(&arrayElementIter);
        }
    }
    dbus_message_unref(reply);
    // free the pending message handle
    dbus_pending_call_unref(pending);
    // free message
    dbus_message_unref(msg);

    get_player_identity(properties->player_name, conn, destination);
    return;

_unref_pending_err:
{
    dbus_pending_call_unref(pending);
    goto _unref_message_err;
}
_unref_message_err:
{
    dbus_message_unref(msg);
}
return;
}

int load_players(DBusConnection* conn, mpris_player *players)
{
    if (NULL == conn) { return 0; }
    if (NULL == players) { return 0; }

    char* method = DBUS_METHOD_LIST_NAMES;
    char* destination = DBUS_DESTINATION;
    char* path = DBUS_PATH;
    char* interface = DBUS_INTERFACE;

    DBusMessage* msg;
    DBusPendingCall* pending;
    int cnt = 0;

    //printf("method: %s dest: %s path: %s interface: %s\n", method, destination, path, interface);

    // create a new method call and check for errors
    msg = dbus_message_new_method_call(destination, path, interface, method);
    if (NULL == msg) { return cnt; }

    // send message and get a handle for a reply
    if (!dbus_connection_send_with_reply (conn, msg, &pending, DBUS_CONNECTION_TIMEOUT) ||
        NULL == pending) {
        goto _unref_message_err;
    }
    dbus_connection_flush(conn);

    // block until we receive a reply
    dbus_pending_call_block(pending);

    DBusMessage* reply = NULL;
    // get the reply message
    reply = dbus_pending_call_steal_reply(pending);
    if (NULL == reply) { goto _unref_pending_err; }

    DBusMessageIter rootIter;
    if (dbus_message_iter_init(reply, &rootIter) && DBUS_TYPE_ARRAY == dbus_message_iter_get_arg_type(&rootIter)) {
        DBusMessageIter arrayElementIter;

        dbus_message_iter_recurse(&rootIter, &arrayElementIter);
        while (cnt < MAX_PLAYERS) {
            if (DBUS_TYPE_STRING == dbus_message_iter_get_arg_type(&arrayElementIter)) {
                char *str = NULL;
                dbus_message_iter_get_basic(&arrayElementIter, &str);
                if (!strncmp(str, MPRIS_PLAYER_NAMESPACE, strlen(MPRIS_PLAYER_NAMESPACE))) {

                    //if (!strncmp(str, "org.mpris.MediaPlayer2.spotifyd", strlen("org.mpris.MediaPlayer2.spotifyd")))
                    //{
                        printf("load players: namespace: %s\n", str);

                        memcpy(players[cnt].namespace, str, strlen(str) + 1);
                        cnt++;
                    //}

                      
                }
            }
            if (!dbus_message_iter_has_next(&arrayElementIter)) {
                break;
            }
            dbus_message_iter_next(&arrayElementIter);
        }
    }
    dbus_message_unref(reply);
_unref_pending_err:
    // free the pending message handle
    dbus_pending_call_unref(pending);
_unref_message_err:
    // free message
    dbus_message_unref(msg);
    return cnt;
}
