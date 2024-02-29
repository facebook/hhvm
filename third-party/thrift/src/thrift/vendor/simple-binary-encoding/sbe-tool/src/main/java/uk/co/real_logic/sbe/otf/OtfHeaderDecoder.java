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
import uk.co.real_logic.sbe.PrimitiveType;
import uk.co.real_logic.sbe.ir.HeaderStructure;
import uk.co.real_logic.sbe.ir.Token;

import java.nio.ByteOrder;

/**
 * Used to decode a message header while doing on-the-fly decoding of a message stream.
 * <p>
 * Metadata is cached to improve the performance of decoding headers.
 * <p>
 * This class is thread safe.
 */
public class OtfHeaderDecoder
{
    private final int length;

    private int blockLengthOffset;
    private int templateIdOffset;
    private int schemaIdOffset;
    private int schemaVersionOffset;
    private PrimitiveType blockLengthType;
    private PrimitiveType templateIdType;
    private PrimitiveType schemaIdType;
    private PrimitiveType schemaVersionType;
    private ByteOrder blockLengthByteOrder;
    private ByteOrder templateIdByteOrder;
    private ByteOrder schemaIdByteOrder;
    private ByteOrder schemaVersionByteOrder;

    /**
     * Read the message header structure and cache the meta data for finding the key fields for decoding messages.
     *
     * @param headerStructure for the meta data describing the message header.
     */
    public OtfHeaderDecoder(final HeaderStructure headerStructure)
    {
        length = headerStructure.tokens().get(0).encodedLength();

        for (final Token token : headerStructure.tokens())
        {
            switch (token.name())
            {
                case HeaderStructure.BLOCK_LENGTH:
                    blockLengthOffset = token.offset();
                    blockLengthType = token.encoding().primitiveType();
                    blockLengthByteOrder = token.encoding().byteOrder();
                    break;

                case HeaderStructure.TEMPLATE_ID:
                    templateIdOffset = token.offset();
                    templateIdType = token.encoding().primitiveType();
                    templateIdByteOrder = token.encoding().byteOrder();
                    break;

                case HeaderStructure.SCHEMA_ID:
                    schemaIdOffset = token.offset();
                    schemaIdType = token.encoding().primitiveType();
                    schemaIdByteOrder = token.encoding().byteOrder();
                    break;

                case HeaderStructure.SCHEMA_VERSION:
                    schemaVersionOffset = token.offset();
                    schemaVersionType = token.encoding().primitiveType();
                    schemaVersionByteOrder = token.encoding().byteOrder();
                    break;
            }
        }
    }

    /**
     * The encodedLength of the message header in bytes.
     *
     * @return the encodedLength of the message header in bytes.
     */
    public int encodedLength()
    {
        return length;
    }

    /**
     * Get the block length of the root block in the message.
     *
     * @param buffer       from which to read the value.
     * @param bufferOffset in the buffer at which the message header begins.
     * @return the length of the root block in the coming message.
     */
    public int getBlockLength(final DirectBuffer buffer, final int bufferOffset)
    {
        return Types.getInt(buffer, bufferOffset + blockLengthOffset, blockLengthType, blockLengthByteOrder);
    }

    /**
     * Get the template id from the message header.
     *
     * @param buffer       from which to read the value.
     * @param bufferOffset in the buffer at which the message header begins.
     * @return the value of the template id.
     */
    public int getTemplateId(final DirectBuffer buffer, final int bufferOffset)
    {
        return Types.getInt(buffer, bufferOffset + templateIdOffset, templateIdType, templateIdByteOrder);
    }

    /**
     * Get the schema id number from the message header.
     *
     * @param buffer       from which to read the value.
     * @param bufferOffset in the buffer at which the message header begins.
     * @return the value of the schema id number.
     */
    public int getSchemaId(final DirectBuffer buffer, final int bufferOffset)
    {
        return Types.getInt(buffer, bufferOffset + schemaIdOffset, schemaIdType, schemaIdByteOrder);
    }

    /**
     * Get the schema version number from the message header.
     *
     * @param buffer       from which to read the value.
     * @param bufferOffset in the buffer at which the message header begins.
     * @return the value of the schema version number.
     */
    public int getSchemaVersion(final DirectBuffer buffer, final int bufferOffset)
    {
        return Types.getInt(buffer, bufferOffset + schemaVersionOffset, schemaVersionType, schemaVersionByteOrder);
    }
}
