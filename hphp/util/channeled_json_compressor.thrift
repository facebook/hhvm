# This file holds definitions for the channeled json protocol.
# Please note that changes to make to this file affect the fb4a repository
# Verify your changes in that repository too.

namespace java com.facebook.http.protocol.channeledjson
namespace cpp HPHP

enum Type {
    TYPE_LONG_MAP = 0,
    TYPE_LONG_ARRAY = 1,
    TYPE_STRING_SIZE = 2,
    TYPE_BOOL_TRUE = 3,
    TYPE_BOOL_FALSE = 4,
    TYPE_LONG_INT = 5,
    TYPE_NONE = 6,
    TYPE_DOUBLE = 7,
    TYPE_MEMOISED_STRING = 8

    TYPE_SMALL_INT_BASE = 9,
    TYPE_SHORT_ARRAY_BASE = 25,
    TYPE_SHORT_MAP_BASE = 41,
    TYPE_SHORT_STRING_BASE = 57

    SHORT_TYPE_LENGTH = 16,
}

enum Channel {
    CHANNEL_TYPES = 0,
    CHANNEL_INTS = 1,
    CHANNEL_STRLEN = 2,
    CHANNEL_MAPLEN = 3,
    CHANNEL_ARRAYLEN = 4,
    CHANNEL_KEYS = 5,
    CHANNEL_STRS = 6,
    CHANNEL_DOUBLES = 7,
    CHANNEL_MEMOSTRS = 8,
    NUM_OF_CHANNELS = 9,
}

enum KeyType {
    KEY_TYPE_STATIC = 0, #Future use.
    KEY_TYPE_STRING = 1,
    KEY_TYPE_MEMOISED_BASE = 2
}


