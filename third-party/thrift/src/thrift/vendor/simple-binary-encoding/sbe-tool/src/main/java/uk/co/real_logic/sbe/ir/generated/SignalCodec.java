/* Generated SBE (Simple Binary Encoding) message codec. */
package uk.co.real_logic.sbe.ir.generated;


/**
 * Token signal type in IR.
 */
@SuppressWarnings("all")
public enum SignalCodec
{

    /**
     * Signal the beginning of a message.
     */
    BEGIN_MESSAGE((short)1),


    /**
     * Signal the end of a message.
     */
    END_MESSAGE((short)2),


    /**
     * Signal the beginning of a composite.
     */
    BEGIN_COMPOSITE((short)3),


    /**
     * Signal the end of a composite.
     */
    END_COMPOSITE((short)4),


    /**
     * Signal the beginning of a field.
     */
    BEGIN_FIELD((short)5),


    /**
     * Signal end beginning of a field.
     */
    END_FIELD((short)6),


    /**
     * Signal the beginning of a group.
     */
    BEGIN_GROUP((short)7),


    /**
     * Signal the end of a group.
     */
    END_GROUP((short)8),


    /**
     * Signal the beginning of a enum.
     */
    BEGIN_ENUM((short)9),


    /**
     * Signal a value of an enum.
     */
    VALID_VALUE((short)10),


    /**
     * Signal the end of a enum.
     */
    END_ENUM((short)11),


    /**
     * Signal the beginning of a set.
     */
    BEGIN_SET((short)12),


    /**
     * Signal the choice in a set.
     */
    CHOICE((short)13),


    /**
     * Signal the end of a set.
     */
    END_SET((short)14),


    /**
     * Signal the beginning of variable data.
     */
    BEGIN_VAR_DATA((short)15),


    /**
     * Signal the end of variable data.
     */
    END_VAR_DATA((short)16),


    /**
     * Signal the encoding of a field.
     */
    ENCODING((short)17),

    /**
     * To be used to represent not present or null.
     */
    NULL_VAL((short)255);

    private final short value;

    SignalCodec(final short value)
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
    public static SignalCodec get(final short value)
    {
        switch (value)
        {
            case 1: return BEGIN_MESSAGE;
            case 2: return END_MESSAGE;
            case 3: return BEGIN_COMPOSITE;
            case 4: return END_COMPOSITE;
            case 5: return BEGIN_FIELD;
            case 6: return END_FIELD;
            case 7: return BEGIN_GROUP;
            case 8: return END_GROUP;
            case 9: return BEGIN_ENUM;
            case 10: return VALID_VALUE;
            case 11: return END_ENUM;
            case 12: return BEGIN_SET;
            case 13: return CHOICE;
            case 14: return END_SET;
            case 15: return BEGIN_VAR_DATA;
            case 16: return END_VAR_DATA;
            case 17: return ENCODING;
            case 255: return NULL_VAL;
        }

        throw new IllegalArgumentException("Unknown value: " + value);
    }
}
