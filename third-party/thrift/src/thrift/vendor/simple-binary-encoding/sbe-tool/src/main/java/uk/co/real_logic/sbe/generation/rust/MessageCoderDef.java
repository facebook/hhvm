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

import uk.co.real_logic.sbe.ir.Ir;
import uk.co.real_logic.sbe.ir.Token;
import uk.co.real_logic.sbe.generation.rust.RustGenerator.CodecType;

import java.io.IOException;
import java.io.Writer;
import java.util.ArrayList;
import java.util.List;

import static uk.co.real_logic.sbe.generation.rust.RustGenerator.CodecType.Encoder;
import static uk.co.real_logic.sbe.generation.rust.RustGenerator.CodecType.Decoder;
import static uk.co.real_logic.sbe.generation.rust.RustGenerator.withLifetime;
import static uk.co.real_logic.sbe.generation.rust.RustUtil.*;

class MessageCoderDef implements RustGenerator.ParentDef
{
    private final StringBuilder sb = new StringBuilder();
    private final ArrayList<SubGroup> subGroups = new ArrayList<>();

    private final Ir ir;
    private final Token msgToken;
    final String name;
    final CodecType codecType;

    MessageCoderDef(final Ir ir, final Token msgToken, final CodecType codecType)
    {
        this.ir = ir;
        this.msgToken = msgToken;
        this.name = formatStructName(msgToken.name());
        this.codecType = codecType;
    }

    String blockLengthType()
    {
        return rustTypeName(ir.headerStructure().blockLengthType());
    }

    String schemaVersionType()
    {
        return rustTypeName(ir.headerStructure().schemaVersionType());
    }

    void generate(
        final List<Token> fields,
        final List<Token> groups,
        final List<Token> varData) throws IOException
    {
        indent(sb, 0, "pub mod %s {\n", codecType.toString().toLowerCase());
        indent(sb, 1, "use super::*;\n\n");

        // i.e. <name>Decoder or <name>Encoder
        final String msgTypeName = formatStructName(msgToken.name()) + codecType.name();
        appendMessageStruct(sb, msgTypeName);

        if (codecType == Encoder)
        {
            RustGenerator.appendImplEncoderTrait(sb, msgTypeName);
        }
        else
        {
            RustGenerator.appendImplDecoderTrait(sb, msgTypeName);
        }

        RustGenerator.appendImplWithLifetimeHeader(sb, msgTypeName); // impl start
        appendWrapFn(sb);

        indent(sb, 2, "#[inline]\n");
        indent(sb, 2, "pub fn encoded_length(&self) -> usize {\n");
        indent(sb, 3, "self.limit - self.offset\n");
        indent(sb, 2, "}\n\n");

        if (codecType == Decoder)
        {
            appendMessageHeaderDecoderFn(sb);

            RustGenerator.generateDecoderFields(sb, fields, 2);
            RustGenerator.generateDecoderGroups(sb, groups, 2, this);
            RustGenerator.generateDecoderVarData(sb, varData, 2, false);
        }
        else
        {
            appendMessageHeaderEncoderFn(sb);

            RustGenerator.generateEncoderFields(sb, fields, 2);
            RustGenerator.generateEncoderGroups(sb, groups, 2, this);
            RustGenerator.generateEncoderVarData(sb, varData, 2);
        }

        indent(sb, 1, "}\n\n"); // impl end

        // append all subGroup generated code
        for (final SubGroup subGroup : subGroups)
        {
            subGroup.appendTo(sb);
        }

        indent(sb, 0, "} // end %s\n\n", codecType.toString().toLowerCase()); // mod end
    }

    void appendTo(final Appendable dest) throws IOException
    {
        dest.append(sb);
    }

    public SubGroup addSubGroup(final String name, final int level, final Token groupToken)
    {
        final SubGroup subGroup = new SubGroup(name, level, groupToken);
        subGroups.add(subGroup);
        return subGroup;
    }

    void appendMessageHeaderEncoderFn(final Appendable out) throws IOException
    {
        indent(out, 2, "pub fn header(self, offset: usize) -> MessageHeaderEncoder<Self> {\n");
        indent(out, 3, "let mut header = MessageHeaderEncoder::default().wrap(self, offset);\n");
        indent(out, 3, "header.block_length(SBE_BLOCK_LENGTH);\n");
        indent(out, 3, "header.template_id(SBE_TEMPLATE_ID);\n");
        indent(out, 3, "header.schema_id(SBE_SCHEMA_ID);\n");
        indent(out, 3, "header.version(SBE_SCHEMA_VERSION);\n");
        indent(out, 3, "header\n");
        indent(out, 2, "}\n\n");
    }

    void appendMessageHeaderDecoderFn(final Appendable out) throws IOException
    {
        indent(out, 2, "pub fn header(self, mut header: MessageHeaderDecoder<ReadBuf<'a>>) -> Self {\n");
        indent(out, 3, "debug_assert_eq!(SBE_TEMPLATE_ID, header.template_id());\n");
        indent(out, 3, "let acting_block_length = header.block_length();\n");
        indent(out, 3, "let acting_version = header.version();\n\n");
        indent(out, 3, "self.wrap(\n");
        indent(out, 4, "header.parent().unwrap(),\n");
        indent(out, 4, "message_header_codec::ENCODED_LENGTH,\n");
        indent(out, 4, "acting_block_length,\n");
        indent(out, 4, "acting_version,\n");
        indent(out, 3, ")\n");
        indent(out, 2, "}\n\n");
    }

    void appendMessageStruct(final Appendable out, final String structName) throws IOException
    {
        if (this.codecType == Decoder)
        {
            indent(out, 1, "#[derive(Clone, Copy, Debug, Default)]\n");
        }
        else
        {
            indent(out, 1, "#[derive(Debug, Default)]\n");
        }

        indent(out, 1, "pub struct %s {\n", withLifetime(structName));
        indent(out, 2, "buf: %s,\n", withLifetime(this.codecType.bufType()));
        indent(out, 2, "initial_offset: usize,\n");
        indent(out, 2, "offset: usize,\n");
        indent(out, 2, "limit: usize,\n");
        if (this.codecType == Decoder)
        {
            indent(out, 2, "pub acting_block_length: %s,\n", blockLengthType());
            indent(out, 2, "pub acting_version: %s,\n", schemaVersionType());
        }
        indent(out, 1, "}\n\n");
    }

    void appendWrapFn(final Appendable out) throws IOException
    {
        if (this.codecType == Decoder)
        {
            indent(out, 2, "pub fn wrap(\n");
            indent(out, 3, "mut self,\n");
            indent(out, 3, "buf: %s,\n", withLifetime(this.codecType.bufType()));
            indent(out, 3, "offset: usize,\n");
            indent(out, 3, "acting_block_length: %s,\n", blockLengthType());
            indent(out, 3, "acting_version: %s,\n", schemaVersionType());
            indent(out, 2, ") -> Self {\n");
            indent(out, 3, "let limit = offset + acting_block_length as usize;\n");
        }
        else
        {
            indent(out, 2, "pub fn wrap(mut self, buf: %s, offset: usize) -> Self {\n",
                withLifetime(this.codecType.bufType()));
            indent(out, 3, "let limit = offset + SBE_BLOCK_LENGTH as usize;\n");
        }

        indent(out, 3, "self.buf = buf;\n");
        indent(out, 3, "self.initial_offset = offset;\n");
        indent(out, 3, "self.offset = offset;\n");
        indent(out, 3, "self.limit = limit;\n");
        if (this.codecType == Decoder)
        {
            indent(out, 3, "self.acting_block_length = acting_block_length;\n");
            indent(out, 3, "self.acting_version = acting_version;\n");
        }
        indent(out, 3, "self\n");
        indent(out, 2, "}\n\n");
    }

    static void generateEncoder(
        final Ir ir,
        final Writer out,
        final Token msgToken,
        final List<Token> fields,
        final List<Token> groups,
        final List<Token> varData) throws IOException
    {
        final MessageCoderDef coderDef = new MessageCoderDef(ir, msgToken, Encoder);
        coderDef.generate(fields, groups, varData);
        coderDef.appendTo(out);
    }

    static void generateDecoder(
        final Ir ir,
        final Writer out,
        final Token msgToken,
        final List<Token> fields,
        final List<Token> groups,
        final List<Token> varData) throws IOException
    {
        final MessageCoderDef coderDef = new MessageCoderDef(ir, msgToken, Decoder);
        coderDef.generate(fields, groups, varData);
        coderDef.appendTo(out);
    }
}
