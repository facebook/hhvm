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
package uk.co.real_logic.sbe.generation.rust;

import org.agrona.Verify;
import uk.co.real_logic.sbe.PrimitiveType;
import uk.co.real_logic.sbe.generation.Generators;
import uk.co.real_logic.sbe.generation.rust.RustGenerator.CodecType;
import uk.co.real_logic.sbe.ir.Encoding;

import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.util.*;

import static java.lang.Long.parseLong;
import static java.lang.String.format;
import static uk.co.real_logic.sbe.generation.Generators.toLowerFirstChar;
import static uk.co.real_logic.sbe.generation.rust.RustGenerator.CodecType.Decoder;
import static uk.co.real_logic.sbe.generation.rust.RustGenerator.CodecType.Encoder;

/**
 * Utility method for Rust codec generation.
 */
public class RustUtil
{
    static final String INDENT = "    ";
    static final Map<PrimitiveType, String> TYPE_NAME_BY_PRIMITIVE_TYPE_MAP = new EnumMap<>(PrimitiveType.class);

    static
    {
        TYPE_NAME_BY_PRIMITIVE_TYPE_MAP.put(PrimitiveType.CHAR, "u8");
        TYPE_NAME_BY_PRIMITIVE_TYPE_MAP.put(PrimitiveType.INT8, "i8");
        TYPE_NAME_BY_PRIMITIVE_TYPE_MAP.put(PrimitiveType.INT16, "i16");
        TYPE_NAME_BY_PRIMITIVE_TYPE_MAP.put(PrimitiveType.INT32, "i32");
        TYPE_NAME_BY_PRIMITIVE_TYPE_MAP.put(PrimitiveType.INT64, "i64");
        TYPE_NAME_BY_PRIMITIVE_TYPE_MAP.put(PrimitiveType.UINT8, "u8");
        TYPE_NAME_BY_PRIMITIVE_TYPE_MAP.put(PrimitiveType.UINT16, "u16");
        TYPE_NAME_BY_PRIMITIVE_TYPE_MAP.put(PrimitiveType.UINT32, "u32");
        TYPE_NAME_BY_PRIMITIVE_TYPE_MAP.put(PrimitiveType.UINT64, "u64");
        TYPE_NAME_BY_PRIMITIVE_TYPE_MAP.put(PrimitiveType.FLOAT, "f32");
        TYPE_NAME_BY_PRIMITIVE_TYPE_MAP.put(PrimitiveType.DOUBLE, "f64");
    }

    /**
     * Map the name of a {@link PrimitiveType} to a Rust primitive type name.
     *
     * @param primitiveType to map.
     * @return the name of the Rust primitive that most closely maps.
     */
    static String rustTypeName(final PrimitiveType primitiveType)
    {
        return TYPE_NAME_BY_PRIMITIVE_TYPE_MAP.get(primitiveType);
    }

    static String generateRustLiteral(final PrimitiveType type, final String value)
    {
        Verify.notNull(type, "type");
        Verify.notNull(value, "value");
        final String typeName = rustTypeName(type);
        if (typeName == null)
        {
            throw new IllegalArgumentException("Unknown Rust type name found for primitive " + type.primitiveName());
        }

        switch (type)
        {
            case CHAR:
            case INT8:
            case INT16:
            case INT32:
            case INT64:
                return value + '_' + typeName;
            case UINT8:
            case UINT16:
            case UINT32:
            case UINT64:
                return "0x" + Long.toHexString(parseLong(value)) + '_' + typeName;
            case FLOAT:
            case DOUBLE:
                return value.endsWith("NaN") ? typeName + "::NAN" : value + '_' + typeName;

            default:
                throw new IllegalArgumentException("Unsupported literal generation for type: " + type.primitiveName());
        }
    }

    static byte eightBitCharacter(final String asciiCharacter)
    {
        Verify.notNull(asciiCharacter, "asciiCharacter");
        final byte[] bytes = asciiCharacter.getBytes(StandardCharsets.US_ASCII);
        if (bytes.length != 1)
        {
            throw new IllegalArgumentException(
                "String value `" + asciiCharacter + "` did not fit into a single 8-bit character");
        }

        return bytes[0];
    }

    static String formatStructName(final String structName)
    {
        return Generators.toUpperFirstChar(structName);
    }

    static String codecModName(final String prefix)
    {
        return toLowerSnakeCase(prefix + "Codec");
    }

    static String codecName(final String structName, final CodecType codecType)
    {
        return formatStructName(structName + codecType.name());
    }

    static String encoderName(final String structName)
    {
        return codecName(structName, Encoder);
    }

    static String decoderName(final String structName)
    {
        return codecName(structName, Decoder);
    }

    static String formatFunctionName(final String value)
    {
        if (value.isEmpty())
        {
            return value;
        }
        return sanitizeMethodOrProperty(toLowerSnakeCase(value));
    }

    static String cleanUpperAcronyms(final String value)
    {
        final int length = value.length();
        for (int i = 0; i < length; i++)
        {
            final char c = value.charAt(i);
            if (!isUpperAlpha(c) && !isNumeric(c))
            {
                if (c != '_' && i > 2)
                {
                    final int index = i - 1;
                    return value.substring(0, index).toLowerCase() + value.substring(index);
                }

                return value;
            }
        }

        return value;
    }

    static String characterEncoding(final Encoding encoding)
    {
        final String characterEncoding = encoding.characterEncoding();
        if (characterEncoding == null)
        {
            return "None";
        }

        return characterEncoding;
    }

    /**
     * Converts to 'snake_case' but will also handle when there are multiple
     * upper case characters in a row 'UPPERCase' => 'upper_case'
     *
     * @param value to be formatted
     * @return the string formatted to 'lower_snake_case'
     */
    static String toLowerSnakeCase(final String value)
    {
        if (value.isEmpty())
        {
            return value;
        }

        final String cleaned = cleanUpperAcronyms(value);
        final String s = toLowerFirstChar(cleaned);
        final int length = s.length();

        final StringBuilder out = new StringBuilder(length + 4);
        char lastChar = '\0';

        for (int i = 0, j = 0; j < length; j++)
        {
            final boolean wasUpper = isUpperAlpha(lastChar);
            final boolean wasNumeric = isNumeric(lastChar);
            final boolean wasUnderscore = lastChar == '_';
            final char c = s.charAt(j);

            if (c == '_')
            {
                out.append(c);
                i = j + 1;
            }
            else if (isUpperAlpha(c))
            {
                if (wasNumeric || (!wasUpper && j - i > 1 && !wasUnderscore))
                {
                    out.append('_');
                    out.append(toLowerSnakeCase(s.substring(j)));
                    return out.toString();
                }
                out.append(Character.toLowerCase(c));
            }
            else if (isNumeric(c))
            {
                if (!wasNumeric && j - i > 1 && !wasUnderscore)
                {
                    out.append('_');
                    out.append(toLowerSnakeCase(s.substring(j)));
                    return out.toString();
                }
                out.append(c);
            }
            else
            {
                if ((wasUpper || wasNumeric) && j - i > 1 && !wasUnderscore)
                {
                    out.append('_');
                    out.append(toLowerSnakeCase(s.substring(j)));
                    return out.toString();
                }
                out.append(c);
            }

            lastChar = c;
        }

        return out.toString();
    }

    private static boolean isUpperAlpha(final char c)
    {
        return 'A' <= c && c <= 'Z';
    }

    private static boolean isNumeric(final char c)
    {
        return '0' <= c && c <= '9';
    }

    private static String sanitizeMethodOrProperty(final String name)
    {
        if (shadowsKeyword(name))
        {
            return "r#" + name;
        }
        else
        {
            return name;
        }
    }

    private static boolean shadowsKeyword(final String name)
    {
        return ReservedKeyword.anyMatch(name);
    }

    static Appendable indent(final Appendable appendable) throws IOException
    {
        return indent(appendable, 1);
    }

    static Appendable indent(final Appendable appendable, final int level) throws IOException
    {
        Appendable out = appendable;
        for (int i = 0; i < level; i++)
        {
            out = out.append(INDENT);
        }

        return out;
    }

    static Appendable indent(final Appendable appendable, final int level, final String f, final Object... args)
        throws IOException
    {
        return indent(appendable, level).append(format(f, args));
    }

    enum ReservedKeyword
    {
        Abstract, AlignOf, As, Async, Become, Box, Break, Const, Continue,
        Crate, Do, Else, Enum, Extern, False, Final, Fn, For, If, Impl, In,
        Let, Loop, Macro, Match, Mod, Move, Mut, OffsetOf, Override, Priv,
        Proc, Pub, Pure, Ref, Return, Self, Sizeof, Static, Struct, Super,
        Trait, True, Type, Typeof, Unsafe, Unsized, Use, Virtual, Where,
        While, Yield;

        private static final Set<String> LOWER_CASE_NAMES = new HashSet<>();

        static
        {
            for (final ReservedKeyword value : ReservedKeyword.values())
            {
                LOWER_CASE_NAMES.add(value.name().toLowerCase());
            }
        }

        static boolean anyMatch(final String v)
        {
            return LOWER_CASE_NAMES.contains(v.toLowerCase());
        }
    }
}
