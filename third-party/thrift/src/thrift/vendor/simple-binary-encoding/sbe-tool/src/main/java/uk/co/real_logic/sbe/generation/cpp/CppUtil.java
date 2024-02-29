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
package uk.co.real_logic.sbe.generation.cpp;

import uk.co.real_logic.sbe.PrimitiveType;
import uk.co.real_logic.sbe.SbeTool;
import uk.co.real_logic.sbe.ValidationUtil;

import java.nio.ByteOrder;
import java.util.EnumMap;
import java.util.Map;

import static uk.co.real_logic.sbe.generation.Generators.toLowerFirstChar;
import static uk.co.real_logic.sbe.generation.Generators.toUpperFirstChar;

/**
 * Utilities for mapping between IR and the C++ language.
 */
public class CppUtil
{
    private static final Map<PrimitiveType, String> PRIMITIVE_TYPE_STRING_ENUM_MAP = new EnumMap<>(PrimitiveType.class);

    static
    {
        PRIMITIVE_TYPE_STRING_ENUM_MAP.put(PrimitiveType.CHAR, "char");
        PRIMITIVE_TYPE_STRING_ENUM_MAP.put(PrimitiveType.INT8, "std::int8_t");
        PRIMITIVE_TYPE_STRING_ENUM_MAP.put(PrimitiveType.INT16, "std::int16_t");
        PRIMITIVE_TYPE_STRING_ENUM_MAP.put(PrimitiveType.INT32, "std::int32_t");
        PRIMITIVE_TYPE_STRING_ENUM_MAP.put(PrimitiveType.INT64, "std::int64_t");
        PRIMITIVE_TYPE_STRING_ENUM_MAP.put(PrimitiveType.UINT8, "std::uint8_t");
        PRIMITIVE_TYPE_STRING_ENUM_MAP.put(PrimitiveType.UINT16, "std::uint16_t");
        PRIMITIVE_TYPE_STRING_ENUM_MAP.put(PrimitiveType.UINT32, "std::uint32_t");
        PRIMITIVE_TYPE_STRING_ENUM_MAP.put(PrimitiveType.UINT64, "std::uint64_t");
        PRIMITIVE_TYPE_STRING_ENUM_MAP.put(PrimitiveType.FLOAT, "float");
        PRIMITIVE_TYPE_STRING_ENUM_MAP.put(PrimitiveType.DOUBLE, "double");
    }

    /**
     * Map the name of a {@link uk.co.real_logic.sbe.PrimitiveType} to a C++98 primitive type name.
     *
     * @param primitiveType to map.
     * @return the name of the Java primitive that most closely maps.
     */
    public static String cppTypeName(final PrimitiveType primitiveType)
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

        if (ValidationUtil.isCppKeyword(formattedValue))
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
     * Format a String as a class name.
     *
     * @param value to be formatted.
     * @return the string formatted as a class name.
     */
    public static String formatClassName(final String value)
    {
        return toUpperFirstChar(value);
    }

    /**
     * Return the Cpp98 formatted byte order encoding string to use for a given byte order and primitiveType
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

    /**
     * Generate a count of closing braces, one on each line.
     *
     * @param count of closing braces.
     * @return A string with count of closing braces.
     */
    public static String closingBraces(final int count)
    {
        final StringBuilder sb = new StringBuilder();
        for (int i = 0; i < count; i++)
        {
            sb.append("}\n");
        }

        return sb.toString();
    }
}
