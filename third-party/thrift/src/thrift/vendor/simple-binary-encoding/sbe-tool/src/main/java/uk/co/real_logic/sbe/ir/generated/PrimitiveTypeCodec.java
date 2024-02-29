/* Generated SBE (Simple Binary Encoding) message codec. */
package uk.co.real_logic.sbe.ir.generated;


/**
 * Primitive types in type system.
 */
@SuppressWarnings("all")
public enum PrimitiveTypeCodec
{

    /**
     * No type is provided.
     */
    NONE((short)0),


    /**
     * Single byte character encoding in ASCII.
     */
    CHAR((short)1),


    /**
     * 8-bit signed integer.
     */
    INT8((short)2),


    /**
     * 16-bit signed integer.
     */
    INT16((short)3),


    /**
     * 32-bit signed integer.
     */
    INT32((short)4),


    /**
     * 64-bit signed integer.
     */
    INT64((short)5),


    /**
     * 8-bit unsigned integer.
     */
    UINT8((short)6),


    /**
     * 16-bit unsigned integer.
     */
    UINT16((short)7),


    /**
     * 32-bit unsigned integer.
     */
    UINT32((short)8),


    /**
     * 64-bit unsigned integer.
     */
    UINT64((short)9),


    /**
     * 32-bit single precision floating point.
     */
    FLOAT((short)10),


    /**
     * 64-bit double precision floating point.
     */
    DOUBLE((short)11),

    /**
     * To be used to represent not present or null.
     */
    NULL_VAL((short)255);

    private final short value;

    PrimitiveTypeCodec(final short value)
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
    public static PrimitiveTypeCodec get(final short value)
    {
        switch (value)
        {
            case 0: return NONE;
            case 1: return CHAR;
            case 2: return INT8;
            case 3: return INT16;
            case 4: return INT32;
            case 5: return INT64;
            case 6: return UINT8;
            case 7: return UINT16;
            case 8: return UINT32;
            case 9: return UINT64;
            case 10: return FLOAT;
            case 11: return DOUBLE;
            case 255: return NULL_VAL;
        }

        throw new IllegalArgumentException("Unknown value: " + value);
    }
}
