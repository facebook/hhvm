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

import org.agrona.concurrent.UnsafeBuffer;
import uk.co.real_logic.sbe.ir.Ir;
import uk.co.real_logic.sbe.ir.Token;
import uk.co.real_logic.sbe.otf.OtfHeaderDecoder;
import uk.co.real_logic.sbe.otf.OtfMessageDecoder;

import java.nio.ByteBuffer;
import java.util.List;

/**
 * Pretty Print JSON based upon the given Ir.
 */
public class JsonPrinter
{
    private final OtfHeaderDecoder headerDecoder;
    private final Ir ir;

    /**
     * Create a new JSON printer for a given message Ir.
     *
     * @param ir for the message type.
     */
    public JsonPrinter(final Ir ir)
    {
        this.ir = ir;
        headerDecoder = new OtfHeaderDecoder(ir.headerStructure());
    }

    /**
     * Print the encoded message to the output.
     *
     * @param encodedMessage with header in buffer.
     * @param output         to write to.
     */
    public void print(final ByteBuffer encodedMessage, final StringBuilder output)
    {
        final UnsafeBuffer buffer = new UnsafeBuffer(encodedMessage);
        print(output, buffer, 0);
    }

    /**
     * Print the encoded message to the output.
     *
     * @param output to write too.
     * @param buffer with encoded message and header.
     * @param offset at which the header begins.
     */
    public void print(final StringBuilder output, final UnsafeBuffer buffer, final int offset)
    {
        final int blockLength = headerDecoder.getBlockLength(buffer, offset);
        final int templateId = headerDecoder.getTemplateId(buffer, offset);
        final int schemaId = headerDecoder.getSchemaId(buffer, offset);
        final int actingVersion = headerDecoder.getSchemaVersion(buffer, offset);

        validateId(schemaId);

        final int messageOffset = offset + headerDecoder.encodedLength();
        final List<Token> msgTokens = ir.getMessage(templateId);

        OtfMessageDecoder.decode(
            buffer,
            messageOffset,
            actingVersion,
            blockLength,
            msgTokens,
            new JsonTokenListener(output));
    }

    /**
     * Print an encoded message to a String.
     *
     * @param encodedMessage with header in buffer.
     * @return encoded message in JSON format.
     */
    public String print(final ByteBuffer encodedMessage)
    {
        final StringBuilder sb = new StringBuilder();
        print(encodedMessage, sb);

        return sb.toString();
    }

    private void validateId(final int schemaId)
    {
        if (schemaId != ir.id())
        {
            throw new IllegalArgumentException("Required schema id " + ir.id() + " but was " + schemaId);
        }
    }
}
