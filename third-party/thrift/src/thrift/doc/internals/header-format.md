# Header format for the THeader.h

      0 1 2 3 4 5 6 7 8 9 a b c d e f 0 1 2 3 4 5 6 7 8 9 a b c d e f
    +----------------------------------------------------------------+
    | 0|                         LENGTH32                            |
    +----------------------------------------------------------------+
    |                           LENGTH_MSW*                          |
    +----------------------------------------------------------------+
    |                           LENGTH_LSW*                          |
    +----------------------------------------------------------------+
    | 0|       HEADER MAGIC          |            FLAGS              |
    +----------------------------------------------------------------+
    |                         SEQUENCE NUMBER                        |
    +----------------------------------------------------------------+
    | 0|     Header Size(/32)        | ...
    +---------------------------------

                      Header is of variable size:
                       (and starts at offset 14)

    +----------------------------------------------------------------+
    |         PROTOCOL ID  (varint)  |   NUM TRANSFORMS (varint)     |
    +----------------------------------------------------------------+
    |      TRANSFORM 0 ID (varint)   |        TRANSFORM 0 DATA ...
    +----------------------------------------------------------------+
    |         ...                              ...                   |
    +----------------------------------------------------------------+
    |        INFO 0 ID (varint)      |       INFO 0  DATA ...
    +----------------------------------------------------------------+
    |         ...                              ...                   |
    +----------------------------------------------------------------+
    |                                                                |
    |                              PAYLOAD                           |
    |                                                                |
    +----------------------------------------------------------------+

The `LENGTH32` field is 32 bits, and counts the remaining bytes in the
packet, NOT including the length field.  For packets 1GiB or larger, the
length field contains the magic value `BIG_FRAME_MAGIC`, and the fields
`LENGTH_MSW` and `LENGTH_LSW` follow and contain the 64-bit length (not
including the `LENGTH32`, `LENGTH_MSW`, and `LENGTH_LSW` fields).  For packets
smaller than 1GiB, the `LENGTH_MSW` and `LENGTH_LSW` fields do not appear, and
the `LENGTH32` field contains the actual length of the packet.  The header size
field is 16 bits, and defines the size of the header remaining NOT including
the `HEADER MAGIC`, `FLAGS`, `SEQUENCE NUMBER` and header size fields.  The
Header size field is in bytes/4.

Note that the `LENGTH_MSW` and `LENGTH_LSW` fields only appear if the
`LENGTH32` field contains the magic value `BIG_FRAME_MAGIC` (0x42494746, "BIGF"
big-endian).

The transform IDs are varints.  The data for each transform is
defined by the transform ID in the code - no size is given in the
header.  If a transform ID is specified from a client and the server
doesn't know about the transform ID, an error MUST be returned as we
don't know how to transform the data.

Conversely, data in the info headers is ignorable.  This should only
be things like timestamps, debugging tracing, etc.  Using the header
size you should be able to skip this data and read the payload safely
if you don't know the info ID.

Info's should be oldest supported to newest supported order, so that
if we read an info ID we don't support, none of the remaining info
ID's will be supported either, and we can safely skip to the payload.

Info ID's and transform ID's should share the same ID space.

### PADDING:

Header will be padded out to next 4-byte boundary with `0x00`.

### Transform IDs:

    ZLIB_TRANSFORM 0x01 - No data for this.  Use zlib to (de)compress the
                          data.

    HMAC_TRANSFORM 0x02 - Deprecated and no longer supported.
    SNAPPY_TRANSFORM  0x03  - Deprecated and no longer supported.


### Info IDs:

    INFO_KEYVALUE 0x01 - varint32 number of headers.
                       - key/value pairs of varstrings (varint16 length plus
                         no-trailing-null string).
