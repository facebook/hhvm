/*
 * Copyright 2013-2024 Real Logic Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package uk.co.real_logic.sbe.generation.c;

import uk.co.real_logic.sbe.PrimitiveType;
import uk.co.real_logic.sbe.SbeTool;
import uk.co.real_logic.sbe.ValidationUtil;

import java.nio.ByteOrder;
import java.util.EnumMap;
import java.util.Map;

import static uk.co.real_logic.sbe.generation.Generators.toLowerFirstChar;

/**
 * Utilities for mapping between IR and the C language.
 */
public class CUtil
{
    private static final Map<PrimitiveType, String> PRIMITIVE_TYPE_STRING_ENUM_MAP = new EnumMap<>(PrimitiveType.class);

    static
    {
        PRIMITIVE_TYPE_STRING_ENUM_MAP.put(PrimitiveType.CHAR, "char");
        PRIMITIVE_TYPE_STRING_ENUM_MAP.put(PrimitiveType.INT8, "int8_t");
        PRIMITIVE_TYPE_STRING_ENUM_MAP.put(PrimitiveType.INT16, "int16_t");
        PRIMITIVE_TYPE_STRING_ENUM_MAP.put(PrimitiveType.INT32, "int32_t");
        PRIMITIVE_TYPE_STRING_ENUM_MAP.put(PrimitiveType.INT64, "int64_t");
        PRIMITIVE_TYPE_STRING_ENUM_MAP.put(PrimitiveType.UINT8, "uint8_t");
        PRIMITIVE_TYPE_STRING_ENUM_MAP.put(PrimitiveType.UINT16, "uint16_t");
        PRIMITIVE_TYPE_STRING_ENUM_MAP.put(PrimitiveType.UINT32, "uint32_t");
        PRIMITIVE_TYPE_STRING_ENUM_MAP.put(PrimitiveType.UINT64, "uint64_t");
        PRIMITIVE_TYPE_STRING_ENUM_MAP.put(PrimitiveType.FLOAT, "float");
        PRIMITIVE_TYPE_STRING_ENUM_MAP.put(PrimitiveType.DOUBLE, "double");
    }

    /**
     * Map the name of a {@link uk.co.real_logic.sbe.PrimitiveType} to a C11 primitive type name.
     *
     * @param primitiveType to map.
     * @return the name of the Java primitive that most closely maps.
     */
    public static String cTypeName(final PrimitiveType primitiveType)
    {
        return PRIMITIVE_TYPE_STRING_ENUM_MAP.get(primitiveType);
    }

    /**
     * Format a String as a property name.
     *
     * @param value to be formatted.
     * @return the string formatted as a property name.
     */
    public static String formatPropertyName(final String value)
    {
        String formattedValue = toLowerFirstChar(value);

        if (ValidationUtil.isCKeyword(formattedValue))
        {
            final String keywordAppendToken = System.getProperty(SbeTool.KEYWORD_APPEND_TOKEN);
            if (null == keywordAppendToken)
            {
                throw new IllegalStateException(
                    "Invalid property name='" + formattedValue +
                    "' please correct the schema or consider setting system property: " + SbeTool.KEYWORD_APPEND_TOKEN);
            }

            formattedValue += keywordAppendToken;
        }

        return formattedValue;
    }

    /**
     * Format a String as a struct name.
     *
     * @param value to be formatted.
     * @return the string formatted as a struct name.
     */
    public static String formatName(final String value)
    {
        return toLowerFirstChar(value);
    }

    /**
     * Format a String as a struct name prepended with a scope.
     *
     * @param scope to be prepended.
     * @param value to be formatted.
     * @return the string formatted as a struct name.
     */
    public static String formatScopedName(final CharSequence[] scope, final String value)
    {
        return String.join("_", scope).toLowerCase() + "_" + formatName(value);
    }

    /**
     * Return the C99 formatted byte order encoding string to use for a given byte order and primitiveType
     *
     * @param byteOrder     of the {@link uk.co.real_logic.sbe.ir.Token}
     * @param primitiveType of the {@link uk.co.real_logic.sbe.ir.Token}
     * @return the string formatted as the byte ordering encoding
     */
    public static String formatByteOrderEncoding(final ByteOrder byteOrder, final PrimitiveType primitiveType)
    {
        switch (primitiveType.size())
        {
            case 2:
                return "SBE_" + byteOrder + "_ENCODE_16";

            case 4:
                return "SBE_" + byteOrder + "_ENCODE_32";

            case 8:
                return "SBE_" + byteOrder + "_ENCODE_64";

            default:
                return "";
        }
    }
}
