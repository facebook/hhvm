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

import uk.co.real_logic.sbe.PrimitiveType;
import uk.co.real_logic.sbe.generation.Generators;
import uk.co.real_logic.sbe.ir.Token;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import static uk.co.real_logic.sbe.generation.rust.RustUtil.indent;
import static uk.co.real_logic.sbe.generation.rust.RustUtil.rustTypeName;

class SubGroup implements RustGenerator.ParentDef
{
    private final StringBuilder sb = new StringBuilder();
    private final ArrayList<SubGroup> subGroups = new ArrayList<>();
    private final String name;
    private final int level;
    private final Token groupToken;

    SubGroup(final String name, final int level, final Token groupToken)
    {
        this.name = name;
        this.level = level;
        this.groupToken = groupToken;
    }

    public SubGroup addSubGroup(final String name, final int level, final Token groupToken)
    {
        final SubGroup subGroup = new SubGroup(name, level, groupToken);
        subGroups.add(subGroup);

        return subGroup;
    }

    void generateEncoder(
        final List<Token> tokens,
        final List<Token> fields,
        final List<Token> groups,
        final List<Token> varData,
        final int index) throws IOException
    {
        final Token blockLengthToken = Generators.findFirst("blockLength", tokens, index);
        final PrimitiveType blockLengthPrimitiveType = blockLengthToken.encoding().primitiveType();

        final Token numInGroupToken = Generators.findFirst("numInGroup", tokens, index);
        final PrimitiveType numInGroupPrimitiveType = numInGroupToken.encoding().primitiveType();

        // define struct...
        indent(sb, level - 1, "#[derive(Debug, Default)]\n");
        indent(sb, level - 1, "pub struct %s<P> {\n", name);
        indent(sb, level, "parent: Option<P>,\n");
        indent(sb, level, "count: %s,\n", rustTypeName(numInGroupPrimitiveType));
        indent(sb, level, "index: usize,\n");
        indent(sb, level, "offset: usize,\n");
        indent(sb, level, "initial_limit: usize,\n");
        indent(sb, level - 1, "}\n\n");

        RustGenerator.appendImplEncoderForComposite(sb, level - 1, name);

        // define impl...
        indent(sb, level - 1, "impl<'a, P> %s<P> where P: Encoder<'a> + Default {\n", name);

        final int dimensionHeaderSize = tokens.get(index).encodedLength();

        // define wrap...
        indent(sb, level, "#[inline]\n");
        indent(sb, level, "pub fn wrap(\n");
        indent(sb, level + 1, "mut self,\n");
        indent(sb, level + 1, "mut parent: P,\n");
        indent(sb, level + 1, "count: %s,\n", rustTypeName(numInGroupPrimitiveType));
        indent(sb, level, ") -> Self {\n");
        indent(sb, level + 1, "let initial_limit = parent.get_limit();\n");
        indent(sb, level + 1, "parent.set_limit(initial_limit + %d);\n", dimensionHeaderSize);
        indent(sb, level + 1, "parent.get_buf_mut().put_%s_at(initial_limit, Self::block_length());\n",
            rustTypeName(blockLengthPrimitiveType));
        indent(sb, level + 1, "parent.get_buf_mut().put_%s_at(initial_limit + %d, count);\n",
            rustTypeName(numInGroupPrimitiveType), numInGroupToken.offset());

        indent(sb, level + 1, "self.parent = Some(parent);\n");
        indent(sb, level + 1, "self.count = count;\n");
        indent(sb, level + 1, "self.index = usize::MAX;\n");
        indent(sb, level + 1, "self.offset = usize::MAX;\n");
        indent(sb, level + 1, "self.initial_limit = initial_limit;\n");
        indent(sb, level + 1, "self\n");
        indent(sb, level, "}\n\n");

        // block_length function
        indent(sb, level, "#[inline]\n");
        indent(sb, level, "pub fn block_length() -> %s {\n", rustTypeName(blockLengthPrimitiveType));
        indent(sb, level + 1, "%d\n", this.groupToken.encodedLength());
        indent(sb, level, "}\n\n");

        // parent function
        indent(sb, level, "#[inline]\n");
        indent(sb, level, "pub fn parent(&mut self) -> SbeResult<P> {\n");
        indent(sb, level + 1, "self.parent.take().ok_or(SbeErr::ParentNotSet)\n");
        indent(sb, level, "}\n\n");

        // advance function...
        indent(sb, level, "/// will return Some(current index) when successful otherwise None\n");
        indent(sb, level, "#[inline]\n");
        indent(sb, level, "pub fn advance(&mut self) -> SbeResult<Option<usize>> {\n");
        indent(sb, level + 1, "let index = self.index.wrapping_add(1);\n");
        indent(sb, level + 1, "if index >= self.count as usize {\n");
        indent(sb, level + 2, "return Ok(None);\n");
        indent(sb, level + 1, "}\n");

        indent(sb, level + 1, "if let Some(parent) = self.parent.as_mut() {\n");
        indent(sb, level + 2, "self.offset = parent.get_limit();\n");
        indent(sb, level + 2, "parent.set_limit(self.offset + Self::block_length() as usize);\n");
        indent(sb, level + 2, "self.index = index;\n");
        indent(sb, level + 2, "Ok(Some(index))\n");
        indent(sb, level + 1, "} else {\n");
        indent(sb, level + 2, "Err(SbeErr::ParentNotSet)\n");
        indent(sb, level + 1, "}\n");
        indent(sb, level, "}\n\n");

        RustGenerator.generateEncoderFields(sb, fields, level);
        RustGenerator.generateEncoderGroups(sb, groups, level, this);
        RustGenerator.generateEncoderVarData(sb, varData, level);

        indent(sb, level - 1, "}\n\n"); // close impl
    }

    void generateDecoder(
        final List<Token> tokens,
        final List<Token> fields,
        final List<Token> groups,
        final List<Token> varData,
        final int index) throws IOException
    {
        final Token blockLengthToken = Generators.findFirst("blockLength", tokens, index);
        final PrimitiveType blockLengthPrimitiveType = blockLengthToken.encoding().primitiveType();

        final Token numInGroupToken = Generators.findFirst("numInGroup", tokens, index);
        final PrimitiveType numInGroupPrimitiveType = numInGroupToken.encoding().primitiveType();

        // define struct...
        indent(sb, level - 1, "#[derive(Debug, Default)]\n");
        indent(sb, level - 1, "pub struct %s<P> {\n", name);
        indent(sb, level, "parent: Option<P>,\n");
        indent(sb, level, "block_length: usize,\n");
        indent(sb, level, "count: %s,\n", rustTypeName(numInGroupPrimitiveType));
        indent(sb, level, "index: usize,\n");
        indent(sb, level, "offset: usize,\n");
        indent(sb, level - 1, "}\n\n");

        RustGenerator.appendImplDecoderForComposite(sb, level - 1, name);

        // define impl...
        indent(sb, level - 1, "impl<'a, P> %s<P> where P: Decoder<'a> + Default {\n", name);

        final int dimensionHeaderSize = tokens.get(index).encodedLength();

        // define wrap...
        indent(sb, level, "pub fn wrap(\n");
        indent(sb, level + 1, "mut self,\n");
        indent(sb, level + 1, "mut parent: P,\n");
        indent(sb, level, ") -> Self {\n");
        indent(sb, level + 1, "let initial_offset = parent.get_limit();\n");
        indent(sb, level + 1, "let block_length = parent.get_buf().get_%s_at(initial_offset) as usize;\n",
            rustTypeName(blockLengthPrimitiveType));
        indent(sb, level + 1, "let count = parent.get_buf().get_%s_at(initial_offset + %d);\n",
            rustTypeName(numInGroupPrimitiveType), numInGroupToken.offset());
        indent(sb, level + 1, "parent.set_limit(initial_offset + %d);\n",
            dimensionHeaderSize);

        indent(sb, level + 1, "self.parent = Some(parent);\n");
        indent(sb, level + 1, "self.block_length = block_length;\n");
        indent(sb, level + 1, "self.count = count;\n");
        indent(sb, level + 1, "self.index = usize::MAX;\n");
        indent(sb, level + 1, "self.offset = 0;\n");
        indent(sb, level + 1, "self\n");
        indent(sb, level, "}\n\n");

        // parent function
        indent(sb, level, "/// group token - %s\n", groupToken);
        indent(sb, level, "#[inline]\n");
        indent(sb, level, "pub fn parent(&mut self) -> SbeResult<P> {\n");
        indent(sb, level + 1, "self.parent.take().ok_or(SbeErr::ParentNotSet)\n");
        indent(sb, level, "}\n\n");

        // count function
        indent(sb, level, "#[inline]\n");
        indent(sb, level, "pub fn count(&self) -> %s {\n", rustTypeName(numInGroupPrimitiveType));
        indent(sb, level + 1, "self.count\n");
        indent(sb, level, "}\n\n");

        // advance function...
        indent(sb, level, "/// will return Some(current index) when successful otherwise None\n");
        indent(sb, level, "pub fn advance(&mut self) -> SbeResult<Option<usize>> {\n");
        indent(sb, level + 1, "let index = self.index.wrapping_add(1);\n");
        indent(sb, level + 1, "if index >= self.count as usize {\n");
        indent(sb, level + 2, " return Ok(None);\n");
        indent(sb, level + 1, "}\n");
        indent(sb, level + 1, "if let Some(parent) = self.parent.as_mut() {\n");
        indent(sb, level + 2, "self.offset = parent.get_limit();\n");
        indent(sb, level + 2, "parent.set_limit(self.offset + self.block_length as usize);\n");
        indent(sb, level + 2, "self.index = index;\n");
        indent(sb, level + 2, "Ok(Some(index))\n");
        indent(sb, level + 1, "} else {\n");
        indent(sb, level + 2, "Err(SbeErr::ParentNotSet)\n");
        indent(sb, level + 1, "}\n");
        indent(sb, level, "}\n\n");

        RustGenerator.generateDecoderFields(sb, fields, level);
        RustGenerator.generateDecoderGroups(sb, groups, level, this);
        RustGenerator.generateDecoderVarData(sb, varData, level, true);

        indent(sb, level - 1, "}\n\n"); // close impl
    }

    void appendTo(final Appendable dest) throws IOException
    {
        dest.append(sb);

        for (final SubGroup subGroup : subGroups)
        {
            subGroup.appendTo(dest);
        }
    }
}
