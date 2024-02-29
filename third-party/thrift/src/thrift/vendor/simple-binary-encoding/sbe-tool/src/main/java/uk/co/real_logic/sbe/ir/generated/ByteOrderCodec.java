/* Generated SBE (Simple Binary Encoding) message codec. */
package uk.co.real_logic.sbe.ir.generated;


/**
 * Number encoding byte order.
 */
@SuppressWarnings("all")
public enum ByteOrderCodec
{

    /**
     * Little Endian byte encoding.
     */
    SBE_LITTLE_ENDIAN((short)0),


    /**
     * Big Endian byte encoding.
     */
    SBE_BIG_ENDIAN((short)1),

    /**
     * To be used to represent not present or null.
     */
    NULL_VAL((short)255);

    private final short value;

    ByteOrderCodec(final short value)
    {
        this.value = value;
    }

    /**
     * The raw encoded value in the Java type representation.
     *
     * @return the raw value encoded.
     */
    public short value()
    {
        return value;
    }

    /**
     * Lookup the enum value representing the value.
     *
     * @param value encoded to be looked up.
     * @return the enum value representing the value.
     */
    public static ByteOrderCodec get(final short value)
    {
        switch (value)
        {
            case 0: return SBE_LITTLE_ENDIAN;
            case 1: return SBE_BIG_ENDIAN;
            case 255: return NULL_VAL;
        }

        throw new IllegalArgumentException("Unknown value: " + value);
    }
}
