/* Generated SBE (Simple Binary Encoding) message codec. */
package uk.co.real_logic.sbe.ir.generated;


/**
 * Field presence declaration.
 */
@SuppressWarnings("all")
public enum PresenceCodec
{

    /**
     * A field is required.
     */
    SBE_REQUIRED((short)0),


    /**
     * A field is optional.
     */
    SBE_OPTIONAL((short)1),


    /**
     * A field is a constant value.
     */
    SBE_CONSTANT((short)2),

    /**
     * To be used to represent not present or null.
     */
    NULL_VAL((short)255);

    private final short value;

    PresenceCodec(final short value)
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
    public static PresenceCodec get(final short value)
    {
        switch (value)
        {
            case 0: return SBE_REQUIRED;
            case 1: return SBE_OPTIONAL;
            case 2: return SBE_CONSTANT;
            case 255: return NULL_VAL;
        }

        throw new IllegalArgumentException("Unknown value: " + value);
    }
}
