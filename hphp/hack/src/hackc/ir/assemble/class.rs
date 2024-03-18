// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use anyhow::Context;
use anyhow::Result;
use ir_core::Class;
use ir_core::ClassName;
use ir_core::CtxConstant;
use ir_core::PropName;
use ir_core::Property;
use ir_core::Requirement;
use ir_core::TraitReqKind;
use ir_core::TypeConstant;
use ir_core::UpperBound;
use parse_macro_ir::parse;

use crate::parse::parse_attr;
use crate::parse::parse_attribute;
use crate::parse::parse_attributes;
use crate::parse::parse_class_name;
use crate::parse::parse_comma_list;
use crate::parse::parse_doc_comment;
use crate::parse::parse_enum;
use crate::parse::parse_hack_constant;
use crate::parse::parse_type_info;
use crate::parse::parse_typed_value;
use crate::parse::parse_user_id;
use crate::parse::parse_visibility;
use crate::tokenizer::Tokenizer;

pub(crate) struct ClassParser {
    class: Class,
}

impl ClassParser {
    pub(crate) fn parse(
        tokenizer: &mut Tokenizer<'_>,
        unit_state: &mut crate::assemble::UnitParser,
    ) -> Result<Class> {
        parse!(tokenizer, <name:parse_user_id> <flags:parse_attr> "{" "\n");

        let name = ClassName::from_utf8(&name.0)?;

        let src_loc = unit_state.get_cur_src_loc();

        let mut state = ClassParser {
            class: Class {
                attributes: Default::default(),
                base: Default::default(),
                constants: Default::default(),
                ctx_constants: Default::default(),
                doc_comment: Default::default(),
                enum_type: Default::default(),
                enum_includes: Default::default(),
                flags,
                implements: Default::default(),
                methods: Default::default(),
                name,
                properties: Default::default(),
                requirements: Default::default(),
                src_loc,
                type_constants: Default::default(),
                upper_bounds: Default::default(),
                uses: Default::default(),
            },
        };

        while !tokenizer.next_is_identifier("}")? {
            let next = tokenizer.expect_any_identifier()?;
            let tok = next.get_identifier().unwrap();
            let res = match tok {
                "attribute" => state.parse_attribute(tokenizer),
                "constant" => state.parse_constant(tokenizer),
                "ctx_constant" => state.parse_ctx_constant(tokenizer),
                "doc_comment" => state.parse_doc_comment(tokenizer),
                "enum_type" => state.parse_enum_type(tokenizer),
                "enum_includes" => state.parse_enum_includes(tokenizer),
                "extends" => state.parse_extends(tokenizer),
                "implements" => state.parse_implements(tokenizer),
                "method" => state.parse_method(tokenizer),
                "property" => state.parse_property(tokenizer),
                "require" => state.parse_require(tokenizer),
                "type_constant" => state.parse_type_constant(tokenizer),
                "upper_bound" => state.parse_upper_bound(tokenizer),
                "uses" => state.parse_uses(tokenizer),
                _ => Err(next.bail(format!("Unexpected token '{next}' parsing class"))),
            };
            res.with_context(|| format!("Parsing {}", next))?;
            tokenizer.expect_eol()?;
        }

        Ok(state.class)
    }

    fn parse_attribute(&mut self, tokenizer: &mut Tokenizer<'_>) -> Result<()> {
        let attr = parse_attribute(tokenizer)?;
        self.class.attributes.push(attr);
        Ok(())
    }

    fn parse_constant(&mut self, tokenizer: &mut Tokenizer<'_>) -> Result<()> {
        let c = parse_hack_constant(tokenizer)?;
        self.class.constants.push(c);
        Ok(())
    }

    fn parse_ctx_constant(&mut self, tokenizer: &mut Tokenizer<'_>) -> Result<()> {
        parse!(tokenizer, <name:parse_user_id>
               "[" <recognized:parse_user_id,*> "]"
               "[" <unrecognized:parse_user_id,*> "]"
               <is_abstract:"abstract"?>);
        let name = ir_core::intern(std::str::from_utf8(&name.0)?);
        let recognized = recognized
            .into_iter()
            .map(|(name, _)| Ok(ir_core::intern(std::str::from_utf8(&name)?)))
            .collect::<Result<_>>()?;
        let unrecognized = unrecognized
            .into_iter()
            .map(|(name, _)| Ok(ir_core::intern(std::str::from_utf8(&name)?)))
            .collect::<Result<_>>()?;
        let is_abstract = is_abstract.is_some();
        let ctx = CtxConstant {
            name,
            recognized,
            unrecognized,
            is_abstract,
        };
        self.class.ctx_constants.push(ctx);
        Ok(())
    }

    fn parse_doc_comment(&mut self, tokenizer: &mut Tokenizer<'_>) -> Result<()> {
        let doc_comment = tokenizer.expect_any_string()?.unescaped_string()?;
        self.class.doc_comment = Some(doc_comment);
        Ok(())
    }

    fn parse_enum_type(&mut self, tokenizer: &mut Tokenizer<'_>) -> Result<()> {
        let ti = parse_type_info(tokenizer)?;
        self.class.enum_type = Some(ti);
        Ok(())
    }

    fn parse_enum_includes(&mut self, tokenizer: &mut Tokenizer<'_>) -> Result<()> {
        parse!(tokenizer, <enum_includes:parse_class_name,*>);
        self.class.enum_includes = enum_includes;
        Ok(())
    }

    fn parse_extends(&mut self, tokenizer: &mut Tokenizer<'_>) -> Result<()> {
        let name = parse_class_name(tokenizer)?;
        self.class.base = Some(name);
        Ok(())
    }

    fn parse_implements(&mut self, tokenizer: &mut Tokenizer<'_>) -> Result<()> {
        let name = parse_class_name(tokenizer)?;
        self.class.implements.push(name);
        Ok(())
    }

    fn parse_method(&mut self, tokenizer: &mut Tokenizer<'_>) -> Result<()> {
        parse!(tokenizer, parse_user_id);
        Ok(())
    }

    fn parse_property(&mut self, tokenizer: &mut Tokenizer<'_>) -> Result<()> {
        parse!(tokenizer,
               <name:parse_user_id> <flags:parse_attr>
               <attributes:parse_attributes("<")> <visibility:parse_visibility>
               ":" <type_info:parse_type_info>
               <doc_comment:parse_doc_comment>);
        let name = PropName::intern(std::str::from_utf8(&name.0)?);

        let initial_value = if tokenizer.next_is_identifier("=")? {
            Some(parse_typed_value(tokenizer)?)
        } else {
            None
        };

        let prop = Property {
            name,
            flags,
            attributes: attributes.into(),
            doc_comment: doc_comment.map(|s| s.into()).into(),
            initial_value: initial_value.into(),
            type_info,
            visibility,
        };
        self.class.properties.push(prop);
        Ok(())
    }

    fn parse_require(&mut self, tokenizer: &mut Tokenizer<'_>) -> Result<()> {
        let kind = parse_enum(tokenizer, "TraitReqKind", |id| {
            Some(match id {
                "extends" => TraitReqKind::MustExtend,
                "implements" => TraitReqKind::MustImplement,
                "must_be_class" => TraitReqKind::MustBeClass,
                _ => return None,
            })
        })?;

        let name = parse_class_name(tokenizer)?;
        self.class.requirements.push(Requirement { name, kind });
        Ok(())
    }

    fn parse_type_constant(&mut self, tokenizer: &mut Tokenizer<'_>) -> Result<()> {
        use ir_core::Maybe;
        parse!(tokenizer, <is_abstract:"abstract"?> <name:parse_user_id>);
        let initializer = if tokenizer.next_is_identifier("=")? {
            Maybe::Just(parse_typed_value(tokenizer)?)
        } else {
            Maybe::Nothing
        };

        let tc = TypeConstant {
            name: ir_core::intern(String::from_utf8(name.0)?),
            initializer,
            is_abstract: is_abstract.is_some(),
        };
        self.class.type_constants.push(tc);
        Ok(())
    }

    fn parse_upper_bound(&mut self, tokenizer: &mut Tokenizer<'_>) -> Result<()> {
        parse!(tokenizer, <name:parse_user_id> ":" "[" <bounds:parse_type_info,*> "]");
        let name = ClassName::intern(std::str::from_utf8(&name.0)?);
        self.class.upper_bounds.push(UpperBound {
            name,
            bounds: bounds.into(),
        });
        Ok(())
    }

    fn parse_uses(&mut self, tokenizer: &mut Tokenizer<'_>) -> Result<()> {
        let name = parse_class_name(tokenizer)?;
        self.class.uses.push(name);
        Ok(())
    }
}
