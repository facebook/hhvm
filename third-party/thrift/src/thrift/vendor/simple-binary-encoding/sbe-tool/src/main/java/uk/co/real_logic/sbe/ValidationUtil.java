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
package uk.co.real_logic.sbe;

import java.util.Arrays;
import java.util.HashSet;
import java.util.Set;
import java.util.regex.Pattern;

/**
 * Various validation utilities used across parser, IR, and generator
 */
public class ValidationUtil
{
    private static final Pattern PATTERN = Pattern.compile("\\.");

    private static final Set<String> C_KEYWORDS = new HashSet<>(Arrays.asList(
        "auto", "_Alignas", "_Alignof", "_Atomic", "bool",
        "_Bool", "break", "case", "_Complex",
        "char", "const", "continue", "default",
        "do", "double", "else", "enum", "extern", "false",
        "float", "for", "_Generic", "goto", "if", "_Imaginary", "inline",
        "int", "long", "_Noreturn", "register", "restrict", "return", "short",
        "signed", "sizeof", "static", "_Static_assert",
        "struct", "switch", "_Thread_local", "true", "typedef", "union",
        "unsigned", "void", "volatile", "wchar_t", "while"));

    /**
     * Check value for validity of usage as a C identifier. A programmatic variable
     * must have all elements be a letter or digit or '_'. The first character must not be a digit.
     * And must not be a C keyword.
     * <p>
     * <a href="http://en.cppreference.com/w/c/keyword">C keywords</a>
     *
     * @param value to check
     * @return true for validity as a C name. false if not.
     */
    public static boolean isSbeCName(final String value)
    {
        if (possibleCKeyword(value))
        {
            if (isCKeyword(value))
            {
                return false;
            }
        }
        else
        {
            return false;
        }

        return true;
    }

    /**
     * Is the token a C language keyword?
     *
     * @param token to be checked.
     * @return true if the token a C language keyword.
     */
    public static boolean isCKeyword(final String token)
    {
        return C_KEYWORDS.contains(token);
    }

    /**
     * Is the value a possible C language keyword?
     *
     * @param value to be checked.
     * @return true if the value is a possible C language keyword.
     */
    private static boolean possibleCKeyword(final String value)
    {
        for (int i = 0, size = value.length(); i < size; i++)
        {
            final char c = value.charAt(i);

            if (i == 0 && isSbeCIdentifierStart(c))
            {
                continue;
            }

            if (isSbeCIdentifierPart(c))
            {
                continue;
            }

            return false;
        }

        return true;
    }

    private static boolean isSbeCIdentifierStart(final char c)
    {
        return Character.isLetter(c) || c == '_';
    }

    private static boolean isSbeCIdentifierPart(final char c)
    {
        return Character.isLetterOrDigit(c) || c == '_';
    }

    private static final Set<String> CPP_KEYWORDS = new HashSet<>(Arrays.asList(
        "alignas", "and", "and_eq", "asm", "auto",
        "bitand", "bitor", "bool", "break", "case",
        "catch", "char", "class", "compl", "const",
        "const_cast", "continue", "char16_t", "char32_t", "default",
        "delete", "do", "double", "dynamic_cast", "else",
        "enum", "explicit", "export", "extern", "false",
        "float", "for", "friend", "goto", "if",
        "inline", "int", "long", "mutable", "namespace",
        "new", "not", "not_eq", "noexcept", "operator",
        "or", "or_eq", "private", "protected", "public",
        "register", "reinterpret_cast", "return", "short", "signed",
        "sizeof", "static", "static_cast", "struct", "switch",
        "template", "this", "throw", "true", "try",
        "typedef", "typeid", "typename", "union", "unsigned",
        "using", "virtual", "void", "volatile", "wchar_t",
        "while", "xor", "xor_eq", "override",
        // since C++11
        "alignof", "constexpr", "decltype", "nullptr", "static_assert", "thread_local",
        // since C++11 have special meaning, so avoid
        "final"));

    /**
     * Check value for validity of usage as a C++ identifier. A programmatic variable
     * must have all elements be a letter or digit or '_'. The first character must not be a digit.
     * And must not be a C++ keyword.
     * <p>
     * <a href="http://en.cppreference.com/w/cpp/keyword">C++ keywords</a>
     *
     * @param value to check
     * @return true for validity as a C++ name. false if not.
     */
    public static boolean isSbeCppName(final String value)
    {
        if (possibleCppKeyword(value))
        {
            if (isCppKeyword(value))
            {
                return false;
            }
        }
        else
        {
            return false;
        }

        return true;
    }

    /**
     * Is the token a C++ language keyword?
     *
     * @param token to be checked.
     * @return true if the token a C++ language keyword.
     */
    public static boolean isCppKeyword(final String token)
    {
        return CPP_KEYWORDS.contains(token);
    }

    /**
     * Is the value a possible C++ language keyword?
     *
     * @param value to be checked.
     * @return true if the value is a possible C++ language keyword.
     */
    private static boolean possibleCppKeyword(final String value)
    {
        for (int i = 0, size = value.length(); i < size; i++)
        {
            final char c = value.charAt(i);

            if (i == 0 && isSbeCppIdentifierStart(c))
            {
                continue;
            }

            if (isSbeCppIdentifierPart(c))
            {
                continue;
            }

            return false;
        }

        return true;
    }

    private static boolean isSbeCppIdentifierStart(final char c)
    {
        return Character.isLetter(c) || c == '_';
    }

    private static boolean isSbeCppIdentifierPart(final char c)
    {
        return Character.isLetterOrDigit(c) || c == '_';
    }

    private static final Set<String> JAVA_KEYWORDS = new HashSet<>(Arrays.asList(
        "abstract", "assert", "boolean", "break", "byte",
        "case", "catch", "char", "class", "const",
        "continue", "default", "do", "double", "else",
        "enum", "extends", "final", "finally", "float",
        "for", "goto", "if", "implements", "import",
        "instanceof", "int", "interface", "long", "native",
        "new", "package", "private", "protected", "public",
        "return", "short", "static", "strictfp", "super",
        "switch", "synchronized", "this", "throw", "throws",
        "transient", "try", "void", "volatile", "while",
        "null", "true", "false", "_"));

    /**
     * Check string for validity of usage as a Java identifier. Avoiding keywords.
     * <p>
     * <a href="http://docs.oracle.com/javase/specs/jls/se8/html/jls-3.html#jls-3.9">Java JLS</a>
     *
     * @param value to check
     * @return true for validity as a Java name. false if not.
     */
    public static boolean isSbeJavaName(final String value)
    {
        for (final String token : PATTERN.split(value, -1))
        {
            if (isJavaIdentifier(token))
            {
                if (isJavaKeyword(token))
                {
                    return false;
                }
            }
            else
            {
                return false;
            }
        }

        return true;
    }

    /**
     * Is this token a Java keyword?
     *
     * @param token to be tested.
     * @return true if a Java keyword otherwise false.
     */
    public static boolean isJavaKeyword(final String token)
    {
        return JAVA_KEYWORDS.contains(token);
    }

    private static boolean isJavaIdentifier(final String token)
    {
        if (token.isEmpty())
        {
            return false;
        }

        for (int i = 0, size = token.length(); i < size; i++)
        {
            final char c = token.charAt(i);

            if (i == 0 && Character.isJavaIdentifierStart(c))
            {
                continue;
            }

            if (Character.isJavaIdentifierPart(c))
            {
                continue;
            }

            return false;
        }

        return true;
    }

    /* https://golang.org/ref/spec#Keywords */
    private static final Set<String> GOLANG_KEYWORDS = new HashSet<>(Arrays.asList(
        "break", "default", "func", "interface", "select",
        "case", "defer", "go", "map", "struct",
        "chan", "else", "goto", "package", "switch",
        "const", "fallthrough", "if", "range", "type",
        "continue", "for", "import", "return", "var",

        /* https://golang.org/ref/spec#Predeclared_identifiers */
        /* types */
        "bool", "byte", "complex64", "complex128", "error", "float32", "float64",
        "int", "int8", "int16", "int32", "int64", "rune", "string",
        "uint", "uint8", "uint16", "uint32", "uint64", "uintptr",
        /* constants */
        "true", "false", "iota",
        /* zero value */
        "nil",
        /* functions */
        "append", "cap", "close", "complex", "copy", "delete", "imag", "len",
        "make", "new", "panic", "print", "println", "real", "recover"));

    /**
     * "Check" value for validity of usage as a golang identifier. From:
     * <a href="https://golang.org/ref/spec#Identifiers">Golang identifiers</a>
     * <p>
     * identifier = letter { letter | unicode_digit }
     * letter        = unicode_letter | "_" .
     * <p>
     * unicode_letter and unicode_digit are defined in section 4.5 of the unicode
     * standard at <a href="http://www.unicode.org/versions/Unicode8.0.0/">Unicode 8.0.0</a> and
     * the Java Character and Digit functions are unicode friendly
     *
     * @param value to check
     * @return true for validity as a golang name. false if not.
     */
    public static boolean isSbeGolangName(final String value)
    {
        if (possibleGolangKeyword(value))
        {
            if (isGolangKeyword(value))
            {
                return false;
            }
        }
        else
        {
            return false;
        }

        return true;
    }

    /**
     * Is the token a Go language keyword?
     *
     * @param token to be checked.
     * @return true if the token a Go language keyword.
     */
    public static boolean isGolangKeyword(final String token)
    {
        return GOLANG_KEYWORDS.contains(token);
    }

    /**
     * Is the value a possible Go language keyword?
     *
     * @param value to be checked.
     * @return true if the value is a possible Go language keyword.
     */
    private static boolean possibleGolangKeyword(final String value)
    {
        for (int i = 0, size = value.length(); i < size; i++)
        {
            final char c = value.charAt(i);

            if (i == 0 && isSbeGolangIdentifierStart(c))
            {
                continue;
            }

            if (isSbeGolangIdentifierPart(c))
            {
                continue;
            }

            return false;
        }

        return true;
    }

    private static boolean isSbeGolangIdentifierStart(final char c)
    {
        return Character.isLetter(c) || c == '_';
    }

    private static boolean isSbeGolangIdentifierPart(final char c)
    {
        return Character.isLetterOrDigit(c) || c == '_';
    }

    /**
     * <a href="https://docs.microsoft.com/en-gb/dotnet/articles/csharp/language-reference/keywords/index">
     *     C# keywords</a>
     * Note this does not include the contextual keywords
     * Note "virtual" is no longer but was in early versions of C#
     */
    private static final Set<String> CSHARP_KEYWORDS = new HashSet<>(Arrays.asList(
        "abstract", "as", "base", "bool", "break",
        "byte", "case", "catch", "char", "checked",
        "class", "const", "continue", "decimal", "default",
        "delegate", "do", "double", "else", "enum",
        "event", "explicit", "extern", "false", "finally",
        "fixed", "float", "for", "foreach", "goto",
        "if", "implicit", "in", "int", "interface",
        "internal", "is", "lock", "long", "namespace",
        "new", "null", "object", "operator", "out",
        "override", "params", "private", "protected", "public",
        "readonly", "ref", "return", "sbyte", "sealed",
        "short", "sizeof", "stackalloc", "static", "string",
        "struct", "switch", "this", "throw", "true",
        "try", "typeof", "uint", "ulong", "unchecked",
        "unsafe", "ushort", "using", "using static", "virtual",
        "void", "volatile", "while"));

    /**
     * "Check" value for validity of usage as a csharp identifier.
     * <a href="https://msdn.microsoft.com/en-us/library/aa664670(v=vs.71).aspx">C# identifiers</a>
     * Which basically boils down to
     * <p>
     * first subsequent*
     * first is { @ | letter | underscore }
     * subsequent is { first | digit | connecting | combining | formatting }*
     * <p>
     * letter is Lu, Ll, Lt, Lm, Lo, or Nl (possibly escaped)
     * digit is Nd (possibly escaped)
     * connecting is Pc (possibly escaped)
     * combining is Mn or Mc (possibly escaped)
     * formatting is Cf (possibly escaped)
     * <p>
     * so that all becomes:
     * { @ | _ | Lu | Ll | Lt | Lm | Lo | Nl } { @ | _ | Lu | Ll | Lt | Lm | Lo | Nl | Nd | Pc | Mn | Mc | Cf}*
     *
     * @param value to check
     * @return true for validity as a csharp name. false if not.
     */
    public static boolean isSbeCSharpName(final String value)
    {
        if (possibleCSharpKeyword(value))
        {
            if (isCSharpKeyword(value))
            {
                return false;
            }
        }
        else
        {
            return false;
        }

        return true;
    }

    /**
     * Is the token a C# language keyword?
     *
     * @param token to be checked.
     * @return true if the token a C# language keyword.
     */
    public static boolean isCSharpKeyword(final String token)
    {
        return CSHARP_KEYWORDS.contains(token);
    }

    /**
     * Is the value a possible C# language keyword?
     *
     * @param value to be checked.
     * @return true if the value is a possible C# language keyword.
     */
    public static boolean possibleCSharpKeyword(final String value)
    {
        for (int i = 0, size = value.length(); i < size; i++)
        {
            final char c = value.charAt(i);

            if (i == 0 && isSbeCSharpIdentifierStart(c))
            {
                continue;
            }

            if (isSbeCSharpIdentifierPart(c))
            {
                continue;
            }

            return false;
        }

        return true;
    }

    private static boolean isSbeCSharpIdentifierStart(final char c)
    {
        return Character.isLetter(c) || c == '_' || c == '@';
    }

    private static boolean isSbeCSharpIdentifierPart(final char c)
    {
        if (isSbeCSharpIdentifierStart(c))
        {
            return true;
        }

        switch (Character.getType(c))
        {
            case Character.NON_SPACING_MARK: // Mn
            case Character.COMBINING_SPACING_MARK: // Mc
            case Character.DECIMAL_DIGIT_NUMBER: // Nd
            case Character.CONNECTOR_PUNCTUATION: // Pc
            case Character.FORMAT: // Cf
                return true;

            default:
                return false;
        }
    }
}
