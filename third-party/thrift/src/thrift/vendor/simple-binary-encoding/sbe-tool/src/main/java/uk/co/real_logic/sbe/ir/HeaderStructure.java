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
package uk.co.real_logic.sbe.ir;

import uk.co.real_logic.sbe.PrimitiveType;
import org.agrona.Verify;

import java.util.List;

/**
 * Metadata description for a message header structure expected before messages to understand the type requiring
 * decoding.
 */
public class HeaderStructure
{
    /**
     * The field containing the length of the root block in bytes.
     */
    public static final String BLOCK_LENGTH = "blockLength";

    /**
     * The field containing the template id of the following message.
     */
    public static final String TEMPLATE_ID = "templateId";

    /**
     * The field containing the schema id to which the following message belongs.
     */
    public static final String SCHEMA_ID = "schemaId";

    /**
     * The field containing the version of the following message.
     */
    public static final String SCHEMA_VERSION = "version";

    private final List<Token> tokens;
    private PrimitiveType blockLengthType;
    private PrimitiveType templateIdType;
    private PrimitiveType schemaIdType;
    private PrimitiveType schemaVersionType;

    /**
     * Construct the header structure from a list of tokens containing the minimum expected set of fields.
     * @param tokens for the header structure.
     */
    public HeaderStructure(final List<Token> tokens)
    {
        Verify.notNull(tokens, "tokens");
        this.tokens = tokens;

        captureEncodings(tokens);

        Verify.notNull(blockLengthType, "blockLengthType");
        Verify.notNull(templateIdType, "templateIdType");
        Verify.notNull(schemaIdType, "schemaIdType");
        Verify.notNull(schemaVersionType, "schemaVersionType");
    }

    /**
     * The IR tokens for the header.
     *
     * @return the IR tokens for the header.
     */
    public List<Token> tokens()
    {
        return tokens;
    }

    /**
     * The declared data type for the block length field.
     *
     * @return the declared data type for the block length field.
     */
    public PrimitiveType blockLengthType()
    {
        return blockLengthType;
    }

    /**
     * The declared data type for the template id field.
     *
     * @return the declared data type for the template id field.
     */
    public PrimitiveType templateIdType()
    {
        return templateIdType;
    }

    /**
     * The declared data type for the SBE schema id field.
     *
     * @return the declared data type for the SBE schema id field.
     */
    public PrimitiveType schemaIdType()
    {
        return schemaIdType;
    }

    /**
     * The declared data type for the SBE schema version field.
     *
     * @return the declared data type for the SBE schema version field.
     */
    public PrimitiveType schemaVersionType()
    {
        return schemaVersionType;
    }

    private void captureEncodings(final List<Token> tokens)
    {
        for (final Token token : tokens)
        {
            switch (token.name())
            {
                case BLOCK_LENGTH:
                    blockLengthType = token.encoding().primitiveType();
                    break;

                case TEMPLATE_ID:
                    templateIdType = token.encoding().primitiveType();
                    break;

                case SCHEMA_ID:
                    schemaIdType = token.encoding().primitiveType();
                    break;

                case SCHEMA_VERSION:
                    schemaVersionType = token.encoding().primitiveType();
                    break;
            }
        }
    }
}
