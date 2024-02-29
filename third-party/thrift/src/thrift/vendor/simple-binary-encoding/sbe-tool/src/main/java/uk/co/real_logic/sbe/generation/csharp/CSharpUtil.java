/*
 * Copyright 2013-2024 Real Logic Limited.
 * Copyright (C) 2017 MarketFactory, Inc
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
package uk.co.real_logic.sbe.generation.csharp;

import uk.co.real_logic.sbe.PrimitiveType;

import java.util.EnumMap;
import java.util.Map;

/**
 * Utilities for mapping between IR and the C# language.
 */
public class CSharpUtil
{
    enum Separators
    {
        BEGIN_GROUP('['),
        END_GROUP(']'),
        BEGIN_COMPOSITE('('),
        END_COMPOSITE(')'),
        BEGIN_SET('{'),
        END_SET('}'),
        BEGIN_ARRAY('['),
        END_ARRAY(']'),
        FIELD('|'),
        KEY_VALUE('='),
        ENTRY(',');

        private final char symbol;

        Separators(final char symbol)
        {
            this.symbol = symbol;
        }

        void appendToGeneratedBuilder(final StringBuilder builder, final String indent)
        {
            builder.append(indent).append("builder.Append('").append(symbol).append("');").append('\n');
        }

        /**
         * Returns the string value of this separator.
         *
         * @return  the string value of this separator
         */
        public String toString()
        {
            return String.valueOf(symbol);
        }
    }

    private static final Map<PrimitiveType, String> PRIMITIVE_TYPE_STRING_ENUM_MAP = new EnumMap<>(PrimitiveType.class);

    /*
     * http://msdn.microsoft.com/en-us/library/ms228360(v=vs.90).aspx
     */
    static
    {
        PRIMITIVE_TYPE_STRING_ENUM_MAP.put(PrimitiveType.CHAR, "byte");
        PRIMITIVE_TYPE_STRING_ENUM_MAP.put(PrimitiveType.INT8, "sbyte");
        PRIMITIVE_TYPE_STRING_ENUM_MAP.put(PrimitiveType.INT16, "short");
        PRIMITIVE_TYPE_STRING_ENUM_MAP.put(PrimitiveType.INT32, "int");
        PRIMITIVE_TYPE_STRING_ENUM_MAP.put(PrimitiveType.INT64, "long");
        PRIMITIVE_TYPE_STRING_ENUM_MAP.put(PrimitiveType.UINT8, "byte");
        PRIMITIVE_TYPE_STRING_ENUM_MAP.put(PrimitiveType.UINT16, "ushort");
        PRIMITIVE_TYPE_STRING_ENUM_MAP.put(PrimitiveType.UINT32, "uint");
        PRIMITIVE_TYPE_STRING_ENUM_MAP.put(PrimitiveType.UINT64, "ulong");
        PRIMITIVE_TYPE_STRING_ENUM_MAP.put(PrimitiveType.FLOAT, "float");
        PRIMITIVE_TYPE_STRING_ENUM_MAP.put(PrimitiveType.DOUBLE, "double");
    }

    /**
     * Map the name of a {@link uk.co.real_logic.sbe.PrimitiveType} to a C# primitive type name.
     *
     * @param primitiveType to map.
     * @return the name of the Java primitive that most closely maps.
     */
    public static String cSharpTypeName(final PrimitiveType primitiveType)
    {
        return PRIMITIVE_TYPE_STRING_ENUM_MAP.get(primitiveType);
    }

    /**
     * Uppercase the first character of a given String.
     *
     * @param str to have the first character upper-cased.
     * @return a new String with the first character in uppercase.
     */
    public static String toUpperFirstChar(final String str)
    {
        return Character.toUpperCase(str.charAt(0)) + str.substring(1);
    }

    /**
     * Lowercase the first character of a given String.
     *
     * @param str to have the first character upper-cased.
     * @return a new String with the first character in uppercase.
     */
    public static String toLowerFirstChar(final String str)
    {
        return Character.toLowerCase(str.charAt(0)) + str.substring(1);
    }

    /**
     * Format a String as a property name.
     *
     * @param str to be formatted.
     * @return the string formatted as a property name.
     */
    public static String formatPropertyName(final String str)
    {
        return toUpperFirstChar(str);
    }

    /**
     * Format a String as a variable name.
     *
     * @param str to be formatted.
     * @return the string formatted as a property name.
     */
    public static String formatVariableName(final String str)
    {
        return toLowerFirstChar(str);
    }

    /**
     * Format a String as a class name.
     *
     * @param str to be formatted.
     * @return the string formatted as a class name.
     */
    public static String formatClassName(final String str)
    {
        return toUpperFirstChar(str);
    }

    /**
     * Format a Getter name for generated code.
     *
     * @param propertyName to be formatted.
     * @return the property name formatted as a getter name.
     */
    public static String formatGetterName(final String propertyName)
    {
        return "Get" + toUpperFirstChar(propertyName);
    }

    /**
     * Shortcut to append a line of generated code
     *
     * @param builder string builder to which to append the line
     * @param indent  current text indentation
     * @param line    line to be appended
     */
    public static void append(final StringBuilder builder, final String indent, final String line)
    {
        builder.append(indent).append(line).append('\n');
    }
}
