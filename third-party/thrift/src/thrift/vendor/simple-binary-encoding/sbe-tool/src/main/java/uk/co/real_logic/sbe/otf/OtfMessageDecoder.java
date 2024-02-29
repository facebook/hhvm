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
package uk.co.real_logic.sbe.otf;

import org.agrona.DirectBuffer;
import uk.co.real_logic.sbe.ir.Token;

import java.util.List;

import static uk.co.real_logic.sbe.ir.Signal.BEGIN_FIELD;
import static uk.co.real_logic.sbe.ir.Signal.BEGIN_GROUP;
import static uk.co.real_logic.sbe.ir.Signal.BEGIN_VAR_DATA;

/**
 * On-the-fly decoder that dynamically decodes messages based on the IR for a schema.
 * <p>
 * The contents of the messages are structurally decomposed and passed to a {@link TokenListener} for decoding the
 * primitive values.
 * <p>
 * The design keeps all state on the stack to maximise performance and avoid object allocation. The message decoder can
 * be reused repeatably by calling {@link OtfMessageDecoder#decode(DirectBuffer, int, int, int, List, TokenListener)}
 * which is thread safe to be used across multiple threads.
 */
@SuppressWarnings("FinalParameters")
public class OtfMessageDecoder
{
    /**
     * Decode a message from the provided buffer based on the message schema described with IR {@link Token}s.
     *
     * @param buffer        containing the encoded message.
     * @param offset        at which the message encoding starts in the buffer.
     * @param actingVersion of the encoded message for dealing with extension fields.
     * @param blockLength   of the root message fields.
     * @param msgTokens     in IR format describing the message structure.
     * @param listener      to callback for decoding the primitive values as discovered in the structure.
     * @return the index in the underlying buffer after decoding.
     */
    public static int decode(
        final DirectBuffer buffer,
        final int offset,
        final int actingVersion,
        final int blockLength,
        final List<Token> msgTokens,
        final TokenListener listener)
    {
        listener.onBeginMessage(msgTokens.get(0));

        int i = offset;
        final int numTokens = msgTokens.size();
        final int tokenIdx = decodeFields(buffer, i, actingVersion, msgTokens, 1, numTokens, listener);
        i += blockLength;

        final long packedValues = decodeGroups(
            buffer, i, actingVersion, msgTokens, tokenIdx, numTokens, listener);

        i = decodeData(
            buffer,
            bufferOffset(packedValues),
            msgTokens,
            tokenIndex(packedValues),
            numTokens,
            actingVersion,
            listener);

        listener.onEndMessage(msgTokens.get(numTokens - 1));

        return i;
    }

    private static int decodeFields(
        final DirectBuffer buffer,
        final int bufferOffset,
        final int actingVersion,
        final List<Token> tokens,
        final int tokenIndex,
        final int numTokens,
        final TokenListener listener)
    {
        int i = tokenIndex;

        while (i < numTokens)
        {
            final Token fieldToken = tokens.get(i);
            if (BEGIN_FIELD != fieldToken.signal())
            {
                break;
            }

            final int nextFieldIdx = i + fieldToken.componentTokenCount();
            i++;

            final Token typeToken = tokens.get(i);
            final int offset = typeToken.offset();

            switch (typeToken.signal())
            {
                case BEGIN_COMPOSITE:
                    decodeComposite(
                        fieldToken,
                        buffer,
                        bufferOffset + offset,
                        tokens, i,
                        nextFieldIdx - 2,
                        actingVersion,
                        listener);
                    break;

                case BEGIN_ENUM:
                    listener.onEnum(
                        fieldToken, buffer, bufferOffset + offset, tokens, i, nextFieldIdx - 2, actingVersion);
                    break;

                case BEGIN_SET:
                    listener.onBitSet(
                        fieldToken, buffer, bufferOffset + offset, tokens, i, nextFieldIdx - 2, actingVersion);
                    break;

                case ENCODING:
                    listener.onEncoding(fieldToken, buffer, bufferOffset + offset, typeToken, actingVersion);
                    break;

                default:
                    break;
            }

            i = nextFieldIdx;
        }

        return i;
    }

    private static long decodeGroups(
        final DirectBuffer buffer,
        int bufferOffset,
        final int actingVersion,
        final List<Token> tokens,
        int tokenIdx,
        final int numTokens,
        final TokenListener listener)
    {
        while (tokenIdx < numTokens)
        {
            final Token token = tokens.get(tokenIdx);
            if (BEGIN_GROUP != token.signal())
            {
                break;
            }

            final boolean isPresent = token.version() <= actingVersion;

            final Token blockLengthToken = tokens.get(tokenIdx + 2);
            final int blockLength = isPresent ? Types.getInt(
                buffer,
                bufferOffset + blockLengthToken.offset(),
                blockLengthToken.encoding().primitiveType(),
                blockLengthToken.encoding().byteOrder()) : 0;

            final Token numInGroupToken = tokens.get(tokenIdx + 3);
            final int numInGroup = isPresent ? Types.getInt(
                buffer,
                bufferOffset + numInGroupToken.offset(),
                numInGroupToken.encoding().primitiveType(),
                numInGroupToken.encoding().byteOrder()) : 0;

            final Token dimensionTypeComposite = tokens.get(tokenIdx + 1);

            if (isPresent)
            {
                bufferOffset += dimensionTypeComposite.encodedLength();
            }

            final int beginFieldsIdx = tokenIdx + dimensionTypeComposite.componentTokenCount() + 1;

            listener.onGroupHeader(token, numInGroup);

            for (int i = 0; i < numInGroup; i++)
            {
                listener.onBeginGroup(token, i, numInGroup);

                final int afterFieldsIdx = decodeFields(
                    buffer, bufferOffset, actingVersion, tokens, beginFieldsIdx, numTokens, listener);
                bufferOffset += blockLength;

                final long packedValues = decodeGroups(
                    buffer, bufferOffset, actingVersion, tokens, afterFieldsIdx, numTokens, listener);

                bufferOffset = decodeData(
                    buffer,
                    bufferOffset(packedValues),
                    tokens,
                    tokenIndex(packedValues),
                    numTokens,
                    actingVersion,
                    listener);

                listener.onEndGroup(token, i, numInGroup);
            }

            tokenIdx += token.componentTokenCount();
        }

        return pack(bufferOffset, tokenIdx);
    }

    private static void decodeComposite(
        final Token fieldToken,
        final DirectBuffer buffer,
        final int bufferOffset,
        final List<Token> tokens,
        final int tokenIdx,
        final int toIndex,
        final int actingVersion,
        final TokenListener listener)
    {
        listener.onBeginComposite(fieldToken, tokens, tokenIdx, toIndex);

        for (int i = tokenIdx + 1; i < toIndex; )
        {
            final Token typeToken = tokens.get(i);
            final int nextFieldIdx = i + typeToken.componentTokenCount();

            final int offset = typeToken.offset();

            switch (typeToken.signal())
            {
                case BEGIN_COMPOSITE:
                    decodeComposite(
                        fieldToken,
                        buffer,
                        bufferOffset + offset,
                        tokens, i,
                        nextFieldIdx - 1,
                        actingVersion,
                        listener);
                    break;

                case BEGIN_ENUM:
                    listener.onEnum(
                        fieldToken, buffer, bufferOffset + offset, tokens, i, nextFieldIdx - 1, actingVersion);
                    break;

                case BEGIN_SET:
                    listener.onBitSet(
                        fieldToken, buffer, bufferOffset + offset, tokens, i, nextFieldIdx - 1, actingVersion);
                    break;

                case ENCODING:
                    listener.onEncoding(typeToken, buffer, bufferOffset + offset, typeToken, actingVersion);
                    break;

                default:
                    break;
            }

            i += typeToken.componentTokenCount();
        }

        listener.onEndComposite(fieldToken, tokens, tokenIdx, toIndex);
    }

    private static int decodeData(
        final DirectBuffer buffer,
        int bufferOffset,
        final List<Token> tokens,
        int tokenIdx,
        final int numTokens,
        final int actingVersion,
        final TokenListener listener)
    {
        while (tokenIdx < numTokens)
        {
            final Token token = tokens.get(tokenIdx);
            if (BEGIN_VAR_DATA != token.signal())
            {
                break;
            }

            final boolean isPresent = token.version() <= actingVersion;

            final Token lengthToken = tokens.get(tokenIdx + 2);
            final int length = isPresent ? Types.getInt(
                buffer,
                bufferOffset + lengthToken.offset(),
                lengthToken.encoding().primitiveType(),
                lengthToken.encoding().byteOrder()) : 0;

            final Token dataToken = tokens.get(tokenIdx + 3);
            if (isPresent)
            {
                bufferOffset += dataToken.offset();
            }

            listener.onVarData(token, buffer, bufferOffset, length, dataToken);

            bufferOffset += length;
            tokenIdx += token.componentTokenCount();
        }

        return bufferOffset;
    }

    private static long pack(final int bufferOffset, final int tokenIndex)
    {
        return ((long)bufferOffset << 32) | tokenIndex;
    }

    private static int bufferOffset(final long packedValues)
    {
        return (int)(packedValues >>> 32);
    }

    private static int tokenIndex(final long packedValues)
    {
        return (int)packedValues;
    }
}
