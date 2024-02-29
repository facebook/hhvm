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
package uk.co.real_logic.sbe.json;

import org.agrona.DirectBuffer;
import org.agrona.PrintBufferUtil;
import uk.co.real_logic.sbe.PrimitiveValue;
import uk.co.real_logic.sbe.ir.Encoding;
import uk.co.real_logic.sbe.ir.Token;
import uk.co.real_logic.sbe.otf.TokenListener;
import uk.co.real_logic.sbe.otf.Types;

import java.io.UnsupportedEncodingException;
import java.util.List;

import static uk.co.real_logic.sbe.PrimitiveType.CHAR;

/**
 * Listener for tokens when dynamically decoding a message which converts them to JSON for output.
 */
public class JsonTokenListener implements TokenListener
{
    private final StringBuilder output;
    private int indentation = 0;
    private int compositeLevel = 0;

    /**
     * Construct a new TokenListener that will write JSON formatted output.
     *
     * @param output to write the JSON formatted output to.
     */
    public JsonTokenListener(final StringBuilder output)
    {
        this.output = output;
    }

    /**
     * {@inheritDoc}
     */
    public void onBeginMessage(final Token token)
    {
        startObject();
    }

    /**
     * {@inheritDoc}
     */
    public void onEndMessage(final Token token)
    {
        endObject();
    }

    /**
     * {@inheritDoc}
     */
    public void onEncoding(
        final Token fieldToken,
        final DirectBuffer buffer,
        final int bufferIndex,
        final Token typeToken,
        final int actingVersion)
    {
        property(compositeLevel > 0 ? typeToken.name() : fieldToken.name());
        appendEncodingAsString(buffer, bufferIndex, fieldToken, typeToken, actingVersion);
        next();
    }

    /**
     * {@inheritDoc}
     */
    public void onEnum(
        final Token fieldToken,
        final DirectBuffer buffer,
        final int bufferIndex,
        final List<Token> tokens,
        final int fromIndex,
        final int toIndex,
        final int actingVersion)
    {
        final Token typeToken = tokens.get(fromIndex + 1);
        final long encodedValue = readEncodingAsLong(buffer, bufferIndex, typeToken, fieldToken, actingVersion);

        String value = null;
        if (fieldToken.isConstantEncoding())
        {
            final String refValue = fieldToken.encoding().constValue().toString();
            final int indexOfDot = refValue.indexOf('.');
            value = -1 == indexOfDot ? refValue : refValue.substring(indexOfDot + 1);
        }
        else
        {
            for (int i = fromIndex + 1; i < toIndex; i++)
            {
                if (encodedValue == tokens.get(i).encoding().constValue().longValue())
                {
                    value = tokens.get(i).name();
                    break;
                }
            }
        }

        property(determineName(0, fieldToken, tokens, fromIndex));
        doubleQuote();
        output.append(value);
        doubleQuote();
        next();
    }

    /**
     * {@inheritDoc}
     */
    public void onBitSet(
        final Token fieldToken,
        final DirectBuffer buffer,
        final int bufferIndex,
        final List<Token> tokens,
        final int fromIndex,
        final int toIndex,
        final int actingVersion)
    {
        final Token typeToken = tokens.get(fromIndex + 1);
        final long encodedValue = readEncodingAsLong(buffer, bufferIndex, typeToken, fieldToken, actingVersion);

        property(determineName(0, fieldToken, tokens, fromIndex));

        output.append("{ ");
        for (int i = fromIndex + 1; i < toIndex; i++)
        {
            output.append('"').append(tokens.get(i).name()).append("\": ");

            final long bitPosition = tokens.get(i).encoding().constValue().longValue();
            final boolean flag = (encodedValue & (1L << bitPosition)) != 0;

            output.append(flag);

            if (i < (toIndex - 1))
            {
                output.append(", ");
            }
        }
        output.append(" }");
        next();
    }

    /**
     * {@inheritDoc}
     */
    public void onBeginComposite(
        final Token fieldToken, final List<Token> tokens, final int fromIndex, final int toIndex)
    {
        ++compositeLevel;
        property(determineName(1, fieldToken, tokens, fromIndex));
        output.append('\n');
        startObject();
    }

    /**
     * {@inheritDoc}
     */
    public void onEndComposite(
        final Token fieldToken, final List<Token> tokens, final int fromIndex, final int toIndex)
    {
        --compositeLevel;
        endObject();
    }

    /**
     * {@inheritDoc}
     */
    public void onGroupHeader(final Token token, final int numInGroup)
    {
        property(token.name());
        if (numInGroup > 0)
        {
            output.append("[\n");
        }
        else
        {
            output.append("[],\n");
        }
    }

    /**
     * {@inheritDoc}
     */
    public void onBeginGroup(final Token token, final int groupIndex, final int numInGroup)
    {
        startObject();
    }

    /**
     * {@inheritDoc}
     */
    public void onEndGroup(final Token token, final int groupIndex, final int numInGroup)
    {
        endObject();
        if (isLastGroup(groupIndex, numInGroup))
        {
            backup();
            output.append("],\n");
        }
    }

    /**
     * {@inheritDoc}
     */
    public void onVarData(
        final Token fieldToken,
        final DirectBuffer buffer,
        final int bufferIndex,
        final int length,
        final Token typeToken)
    {
        try
        {
            property(fieldToken.name());
            doubleQuote();

            final byte[] tempBuffer = new byte[length];
            buffer.getBytes(bufferIndex, tempBuffer, 0, length);

            final String charsetName = typeToken.encoding().characterEncoding();
            final String str = charsetName != null ? new String(tempBuffer, 0, length, charsetName) :
                PrintBufferUtil.hexDump(tempBuffer);

            escape(str);

            doubleQuote();
            next();
        }
        catch (final UnsupportedEncodingException ex)
        {
            ex.printStackTrace();
        }
    }

    private static boolean isLastGroup(final int groupIndex, final int numInGroup)
    {
        return groupIndex == numInGroup - 1;
    }

    private void next()
    {
        output.append(",\n");
    }

    private void property(final String name)
    {
        indent();
        doubleQuote();
        output.append(name);
        output.append("\": ");
    }

    private void appendEncodingAsString(
        final DirectBuffer buffer,
        final int index,
        final Token fieldToken,
        final Token typeToken,
        final int actingVersion)
    {
        final int arrayLength = typeToken.arrayLength();
        final Encoding encoding = typeToken.encoding();
        final PrimitiveValue constOrNotPresentValue = constOrNotPresentValue(typeToken, fieldToken, actingVersion);

        if (null != constOrNotPresentValue)
        {
            final String characterEncoding = encoding.characterEncoding();
            if (null != characterEncoding)
            {
                doubleQuote();

                if (PrimitiveValue.Representation.LONG == constOrNotPresentValue.representation())
                {
                    try
                    {
                        final long longValue = constOrNotPresentValue.longValue();
                        if (PrimitiveValue.NULL_VALUE_CHAR != longValue)
                        {
                            escape(new String(new byte[]{ (byte)longValue }, characterEncoding));
                        }
                    }
                    catch (final UnsupportedEncodingException ex)
                    {
                        throw new RuntimeException(ex);
                    }
                }
                else
                {
                    escape(constOrNotPresentValue.toString());
                }

                doubleQuote();
            }
            else
            {
                if (arrayLength < 2)
                {
                    Types.appendAsJsonString(output, constOrNotPresentValue, encoding);
                }
                else
                {
                    output.append('[');

                    for (int i = 0; i < arrayLength; i++)
                    {
                        if (i > 0)
                        {
                            output.append(", ");
                        }
                        Types.appendAsJsonString(output, constOrNotPresentValue, encoding);
                    }

                    output.append(']');
                }
            }
        }
        else
        {
            final int elementSize = encoding.primitiveType().size();
            final int size = typeToken.arrayLength();

            if (size > 1 && encoding.primitiveType() == CHAR)
            {
                doubleQuote();
                escapePrintableChar(buffer, index, size, elementSize);
                doubleQuote();
            }
            else
            {
                if (1 == size)
                {
                    Types.appendAsJsonString(output, buffer, index, encoding);
                }
                else
                {
                    output.append('[');

                    for (int i = 0; i < size; i++)
                    {
                        if (i > 0)
                        {
                            output.append(", ");
                        }
                        Types.appendAsJsonString(output, buffer, index + (i * elementSize), encoding);
                    }

                    output.append(']');
                }
            }
        }
    }

    private void escapePrintableChar(final DirectBuffer buffer, final int index, final int size, final int elementSize)
    {
        for (int i = 0; i < size; i++)
        {
            final byte c = buffer.getByte(index + (i * elementSize));
            if (c > 0)
            {
                escape((char)c);
            }
            else
            {
                break;
            }
        }
    }

    private void backup()
    {
        final int newLength = output.length() - 2;
        if (output.charAt(newLength) == ',')
        {
            output.setLength(newLength);
        }
    }

    private void indent()
    {
        for (int i = 0; i < indentation; i++)
        {
            output.append("    ");
        }
    }

    private void doubleQuote()
    {
        output.append('\"');
    }

    private void startObject()
    {
        indent();
        output.append("{\n");
        indentation++;
    }

    private void endObject()
    {
        backup();
        output.append('\n');
        indentation--;
        indent();
        output.append('}');

        if (indentation > 0)
        {
            next();
        }
    }

    private String determineName(
        final int thresholdLevel, final Token fieldToken, final List<Token> tokens, final int fromIndex)
    {
        if (compositeLevel > thresholdLevel)
        {
            return tokens.get(fromIndex).name();
        }
        else
        {
            return fieldToken.name();
        }
    }

    private static PrimitiveValue constOrNotPresentValue(
        final Token typeToken, final Token fieldToken, final int actingVersion)
    {
        final Encoding encoding = typeToken.encoding();
        if (typeToken.isConstantEncoding())
        {
            return encoding.constValue();
        }
        else if (fieldToken.isOptionalEncoding() && actingVersion < fieldToken.version())
        {
            return encoding.applicableNullValue();
        }

        return null;
    }

    private static long readEncodingAsLong(
        final DirectBuffer buffer,
        final int bufferIndex,
        final Token typeToken,
        final Token fieldToken,
        final int actingVersion)
    {
        final PrimitiveValue constOrNotPresentValue = constOrNotPresentValue(typeToken, fieldToken, actingVersion);
        if (null != constOrNotPresentValue)
        {
            return constOrNotPresentValue.longValue();
        }

        return Types.getLong(buffer, bufferIndex, typeToken.encoding());
    }

    private void escape(final String str)
    {
        for (int i = 0, length = str.length(); i < length; i++)
        {
            escape(str.charAt(i));
        }
    }

    private void escape(final char c)
    {
        if ('"' == c || '\\' == c || '\b' == c || '\f' == c || '\n' == c || '\r' == c || '\t' == c)
        {
            output.append('\\');
        }

        output.append(c);
    }
}
