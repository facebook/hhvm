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
package uk.co.real_logic.sbe.examples;

import org.agrona.DirectBuffer;
import uk.co.real_logic.sbe.PrimitiveValue;
import uk.co.real_logic.sbe.ir.Encoding;
import uk.co.real_logic.sbe.ir.Token;
import uk.co.real_logic.sbe.otf.TokenListener;
import uk.co.real_logic.sbe.otf.Types;

import java.io.PrintWriter;
import java.io.UnsupportedEncodingException;
import java.util.ArrayDeque;
import java.util.Deque;
import java.util.Iterator;
import java.util.List;

/**
 * Example of a {@link TokenListener} implementation which prints a decoded message to a {@link PrintWriter}.
 */
public class ExampleTokenListener implements TokenListener
{
    private int compositeLevel = 0;
    private final PrintWriter out;
    private final Deque<String> namedScope = new ArrayDeque<>();
    private final byte[] tempBuffer = new byte[1024];

    /**
     * Construct an example {@link TokenListener} that prints tokens out.
     *
     * @param out to print the tokens to.
     */
    public ExampleTokenListener(final PrintWriter out)
    {
        this.out = out;
    }

    /**
     * {@inheritDoc}
     */
    public void onBeginMessage(final Token token)
    {
        namedScope.push(token.name() + ".");
    }

    /**
     * {@inheritDoc}
     */
    public void onEndMessage(final Token token)
    {
        namedScope.pop();
    }

    /**
     * {@inheritDoc}
     */
    public void onEncoding(
        final Token fieldToken,
        final DirectBuffer buffer,
        final int index,
        final Token typeToken,
        final int actingVersion)
    {
        final CharSequence value = readEncodingAsString(buffer, index, typeToken, fieldToken.version(), actingVersion);

        printScope();
        out.append(compositeLevel > 0 ? typeToken.name() : fieldToken.name())
            .append('=')
            .append(value)
            .println();
    }

    /**
     * {@inheritDoc}
     */
    public void onEnum(
        final Token fieldToken,
        final DirectBuffer buffer,
        final int bufferIndex,
        final List<Token> tokens,
        final int beginIndex,
        final int endIndex,
        final int actingVersion)
    {
        final Token typeToken = tokens.get(beginIndex + 1);
        final long encodedValue = readEncodingAsLong(
            buffer, bufferIndex, typeToken, fieldToken.version(), actingVersion);

        String value = null;
        if (fieldToken.isConstantEncoding())
        {
            final String refValue = fieldToken.encoding().constValue().toString();
            final int indexOfDot = refValue.indexOf('.');
            value = -1 == indexOfDot ? refValue : refValue.substring(indexOfDot + 1);
        }
        else
        {
            for (int i = beginIndex + 1; i < endIndex; i++)
            {
                if (encodedValue == tokens.get(i).encoding().constValue().longValue())
                {
                    value = tokens.get(i).name();
                    break;
                }
            }
        }

        printScope();
        out.append(determineName(0, fieldToken, tokens, beginIndex))
            .append('=')
            .append(value)
            .println();
    }

    /**
     * {@inheritDoc}
     */
    public void onBitSet(
        final Token fieldToken,
        final DirectBuffer buffer,
        final int bufferIndex,
        final List<Token> tokens,
        final int beginIndex,
        final int endIndex,
        final int actingVersion)
    {
        final Token typeToken = tokens.get(beginIndex + 1);
        final long encodedValue = readEncodingAsLong(
            buffer, bufferIndex, typeToken, fieldToken.version(), actingVersion);

        printScope();
        out.append(determineName(0, fieldToken, tokens, beginIndex)).append(':');

        for (int i = beginIndex + 1; i < endIndex; i++)
        {
            out.append(' ').append(tokens.get(i).name()).append('=');

            final long bitPosition = tokens.get(i).encoding().constValue().longValue();
            final boolean flag = (encodedValue & (1L << bitPosition)) != 0;

            out.append(Boolean.toString(flag));
        }

        out.println();
    }

    /**
     * {@inheritDoc}
     */
    public void onBeginComposite(
        final Token fieldToken, final List<Token> tokens, final int fromIndex, final int toIndex)
    {
        ++compositeLevel;
        namedScope.push(determineName(1, fieldToken, tokens, fromIndex) + ".");
    }

    /**
     * {@inheritDoc}
     */
    public void onEndComposite(final Token fieldToken, final List<Token> tokens, final int fromIndex, final int toIndex)
    {
        --compositeLevel;
        namedScope.pop();
    }

    /**
     * {@inheritDoc}
     */
    public void onGroupHeader(final Token token, final int numInGroup)
    {
        printScope();
        out.append(token.name())
            .append(" Group Header : numInGroup=")
            .append(Integer.toString(numInGroup))
            .println();
    }

    /**
     * {@inheritDoc}
     */
    public void onBeginGroup(final Token token, final int groupIndex, final int numInGroup)
    {
        namedScope.push(token.name() + ".");
    }

    /**
     * {@inheritDoc}
     */
    public void onEndGroup(final Token token, final int groupIndex, final int numInGroup)
    {
        namedScope.pop();
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
        final String value;
        try
        {
            final String characterEncoding = typeToken.encoding().characterEncoding();
            if (null == characterEncoding)
            {
                value = length + " bytes of raw data";
            }
            else
            {
                buffer.getBytes(bufferIndex, tempBuffer, 0, length);
                value = new String(tempBuffer, 0, length, characterEncoding);
            }
        }
        catch (final UnsupportedEncodingException ex)
        {
            ex.printStackTrace();
            return;
        }

        printScope();
        out.append(fieldToken.name())
            .append('=')
            .append(value)
            .println();
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

    private static CharSequence readEncodingAsString(
        final DirectBuffer buffer,
        final int index,
        final Token typeToken,
        final int fieldVersion,
        final int actingVersion)
    {
        final PrimitiveValue constOrNotPresentValue = constOrNotPresentValue(typeToken, fieldVersion, actingVersion);
        if (null != constOrNotPresentValue)
        {
            final String characterEncoding = constOrNotPresentValue.characterEncoding();
            if (constOrNotPresentValue.size() == 1 && null != characterEncoding)
            {
                try
                {
                    final byte[] bytes = { (byte)constOrNotPresentValue.longValue() };
                    return new String(bytes, characterEncoding);
                }
                catch (final UnsupportedEncodingException ex)
                {
                    throw new RuntimeException(ex);
                }
            }
            else
            {
                final String value = constOrNotPresentValue.toString();
                final int size = typeToken.arrayLength();

                if (size < 2)
                {
                    return value;
                }

                final StringBuilder sb = new StringBuilder();

                for (int i = 0; i < size; i++)
                {
                    sb.append(value).append(", ");
                }

                sb.setLength(sb.length() - 2);

                return sb;
            }
        }

        final StringBuilder sb = new StringBuilder();
        final Encoding encoding = typeToken.encoding();
        final int elementSize = encoding.primitiveType().size();

        for (int i = 0, size = typeToken.arrayLength(); i < size; i++)
        {
            Types.appendAsString(sb, buffer, index + (i * elementSize), encoding);
            sb.append(", ");
        }

        sb.setLength(sb.length() - 2);

        return sb;
    }

    private static long readEncodingAsLong(
        final DirectBuffer buffer,
        final int bufferIndex,
        final Token typeToken,
        final int fieldVersion,
        final int actingVersion)
    {
        final PrimitiveValue constOrNotPresentValue = constOrNotPresentValue(typeToken, fieldVersion, actingVersion);
        if (null != constOrNotPresentValue)
        {
            return constOrNotPresentValue.longValue();
        }

        return Types.getLong(buffer, bufferIndex, typeToken.encoding());
    }

    private static PrimitiveValue constOrNotPresentValue(
        final Token typeToken, final int fieldVersion, final int actingVersion)
    {
        if (typeToken.isConstantEncoding())
        {
            return typeToken.encoding().constValue();
        }
        else if (typeToken.isOptionalEncoding() && actingVersion < fieldVersion)
        {
            return typeToken.encoding().applicableNullValue();
        }

        return null;
    }

    private void printScope()
    {
        final Iterator<String> i = namedScope.descendingIterator();
        while (i.hasNext())
        {
            out.print(i.next());
        }
    }
}
