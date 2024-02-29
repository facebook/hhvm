/*
 * Copyright 2013-2024 Real Logic Limited.
 * Copyright (C) 2016 MarketFactory, Inc
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
package uk.co.real_logic.sbe.generation.golang;

import uk.co.real_logic.sbe.PrimitiveType;
import uk.co.real_logic.sbe.SbeTool;
import uk.co.real_logic.sbe.ValidationUtil;

import java.util.EnumMap;
import java.util.Map;

import static uk.co.real_logic.sbe.generation.Generators.toUpperFirstChar;

/**
 * Utilities for mapping between IR and the Golang language.
 */
public class GolangUtil
{
    private static final Map<PrimitiveType, String> PRIMITIVE_TYPE_STRING_ENUM_MAP = new EnumMap<>(PrimitiveType.class);

    static
    {
        PRIMITIVE_TYPE_STRING_ENUM_MAP.put(PrimitiveType.CHAR, "byte");
        PRIMITIVE_TYPE_STRING_ENUM_MAP.put(PrimitiveType.INT8, "int8");
        PRIMITIVE_TYPE_STRING_ENUM_MAP.put(PrimitiveType.INT16, "int16");
        PRIMITIVE_TYPE_STRING_ENUM_MAP.put(PrimitiveType.INT32, "int32");
        PRIMITIVE_TYPE_STRING_ENUM_MAP.put(PrimitiveType.INT64, "int64");
        PRIMITIVE_TYPE_STRING_ENUM_MAP.put(PrimitiveType.UINT8, "uint8");
        PRIMITIVE_TYPE_STRING_ENUM_MAP.put(PrimitiveType.UINT16, "uint16");
        PRIMITIVE_TYPE_STRING_ENUM_MAP.put(PrimitiveType.UINT32, "uint32");
        PRIMITIVE_TYPE_STRING_ENUM_MAP.put(PrimitiveType.UINT64, "uint64");
        PRIMITIVE_TYPE_STRING_ENUM_MAP.put(PrimitiveType.FLOAT, "float32");
        PRIMITIVE_TYPE_STRING_ENUM_MAP.put(PrimitiveType.DOUBLE, "float64");
    }

    /**
     * Map the name of a {@link uk.co.real_logic.sbe.PrimitiveType} to a Golang primitive type name.
     *
     * @param primitiveType to map.
     * @return the name of the Java primitive that most closely maps.
     */
    public static String golangTypeName(final PrimitiveType primitiveType)
    {
        return PRIMITIVE_TYPE_STRING_ENUM_MAP.get(primitiveType);
    }

    private static final Map<PrimitiveType, String> MARSHAL_TYPE_BY_PRIMITIVE_TYPE_MAP =
        new EnumMap<>(PrimitiveType.class);

    static
    {
        MARSHAL_TYPE_BY_PRIMITIVE_TYPE_MAP.put(PrimitiveType.CHAR, "Bytes");
        MARSHAL_TYPE_BY_PRIMITIVE_TYPE_MAP.put(PrimitiveType.INT8, "Int8");
        MARSHAL_TYPE_BY_PRIMITIVE_TYPE_MAP.put(PrimitiveType.INT16, "Int16");
        MARSHAL_TYPE_BY_PRIMITIVE_TYPE_MAP.put(PrimitiveType.INT32, "Int32");
        MARSHAL_TYPE_BY_PRIMITIVE_TYPE_MAP.put(PrimitiveType.INT64, "Int64");
        MARSHAL_TYPE_BY_PRIMITIVE_TYPE_MAP.put(PrimitiveType.UINT8, "Uint8");
        MARSHAL_TYPE_BY_PRIMITIVE_TYPE_MAP.put(PrimitiveType.UINT16, "Uint16");
        MARSHAL_TYPE_BY_PRIMITIVE_TYPE_MAP.put(PrimitiveType.UINT32, "Uint32");
        MARSHAL_TYPE_BY_PRIMITIVE_TYPE_MAP.put(PrimitiveType.UINT64, "Uint64");
        MARSHAL_TYPE_BY_PRIMITIVE_TYPE_MAP.put(PrimitiveType.FLOAT, "Float32");
        MARSHAL_TYPE_BY_PRIMITIVE_TYPE_MAP.put(PrimitiveType.DOUBLE, "Float64");
    }

    /**
     * Map the name of a {@link uk.co.real_logic.sbe.PrimitiveType} to a Golang marhsalling function name.
     *
     * @param primitiveType to map.
     * @return the name of the Java primitive that most closely maps.
     */
    public static String golangMarshalType(final PrimitiveType primitiveType)
    {
        return MARSHAL_TYPE_BY_PRIMITIVE_TYPE_MAP.get(primitiveType);
    }

    /**
     * Format a String as a property name.
     *
     * @param value to be formatted.
     * @return the string formatted as a property name.
     */
    public static String formatPropertyName(final String value)
    {
        String formattedValue = toUpperFirstChar(value);

        if (ValidationUtil.isGolangKeyword(formattedValue))
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
     * Format a String as a type name.
     *
     * @param value to be formatted.
     * @return the string formatted as an exported type name.
     */
    public static String formatTypeName(final String value)
    {
        return toUpperFirstChar(value);
    }
}
