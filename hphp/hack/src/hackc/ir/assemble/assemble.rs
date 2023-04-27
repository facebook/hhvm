// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::io::Read;
use std::path::Path;
use std::sync::Arc;

use anyhow::Context;
use anyhow::Result;
use bumpalo::Bump;
use ir_core::ClassId;
use ir_core::ClassName;
use ir_core::ConstName;
use ir_core::Fatal;
use ir_core::Function;
use ir_core::FunctionName;
use ir_core::IncludePath;
use ir_core::Method;
use ir_core::MethodId;
use ir_core::Module;
use ir_core::SrcLoc;
use ir_core::StringInterner;
use ir_core::Typedef;
use ir_core::Unit;
use parse_macro_ir::parse;

use crate::parse::parse_attr;
use crate::parse::parse_attribute;
use crate::parse::parse_attributes;
use crate::parse::parse_class_id;
use crate::parse::parse_comma_list;
use crate::parse::parse_doc_comment;
use crate::parse::parse_fatal_op;
use crate::parse::parse_hack_constant;
use crate::parse::parse_src_loc;
use crate::parse::parse_type_info;
use crate::parse::parse_typed_value;
use crate::parse::parse_user_id;
use crate::tokenizer::Tokenizer;

pub fn unit_from_path<'a>(
    path: &Path,
    strings: Arc<StringInterner>,
    alloc: &'a Bump,
) -> Result<Unit<'a>> {
    use std::fs::File;
    let mut file = File::open(path)?;
    read_unit(&mut file, &format!("{}", path.display()), strings, alloc)
}

pub fn unit_from_string<'a>(
    input: &str,
    strings: Arc<StringInterner>,
    alloc: &'a Bump,
) -> Result<Unit<'a>> {
    let mut input = input.as_bytes();
    read_unit(&mut input, "<string>", strings, alloc)
}

pub fn read_unit<'a>(
    read: &mut dyn Read,
    filename: &str,
    strings: Arc<StringInterner>,
    alloc: &'a Bump,
) -> Result<Unit<'a>> {
    let mut tokenizer = Tokenizer::new(read, filename, strings);

    let unit = UnitParser::parse(&mut tokenizer, alloc)?;
    Ok(unit)
}

pub(crate) struct UnitParser<'a> {
    pub(crate) alloc: &'a Bump,
    pub(crate) unit: Unit<'a>,
    pub(crate) src_loc: Option<SrcLoc>,
}

impl<'a> UnitParser<'a> {
    pub(crate) fn get_cur_src_loc(&self) -> SrcLoc {
        self.src_loc.as_ref().cloned().unwrap_or_default()
    }
}

impl<'a> UnitParser<'a> {
    fn parse(tokenizer: &mut Tokenizer<'_>, alloc: &'a Bump) -> Result<Unit<'a>> {
        let strings = Arc::clone(&tokenizer.strings);

        let mut state = UnitParser {
            alloc,
            unit: Unit {
                strings,
                ..Default::default()
            },
            src_loc: None,
        };

        while let Some(next) = tokenizer.read_token()? {
            if next.is_eol() {
                continue;
            }
            let res = match next.identifier() {
                ".class_ref" => state.parse_class_ref(tokenizer),
                ".const_ref" => state.parse_const_ref(tokenizer),
                ".fatal" => state.parse_fatal(tokenizer),
                ".func_ref" => state.parse_func_ref(tokenizer),
                ".include_ref" => state.parse_include_ref(tokenizer),
                ".srcloc" => state.parse_src_loc(tokenizer),
                "attribute" => state.parse_file_attribute(tokenizer),
                "class" => state.parse_class(tokenizer),
                "constant" => state.parse_constant(tokenizer),
                "function" => state.parse_function(tokenizer),
                "method" => state.parse_method(tokenizer),
                "module" => state.parse_module(tokenizer),
                "module_use" => state.parse_module_use(tokenizer),
                "typedef" => state.parse_typedef(tokenizer),
                _ => Err(next.bail(format!("Unexpected token '{next}' in input"))),
            };
            res.with_context(|| format!("Parsing {}", next))?;
        }

        Ok(state.unit)
    }

    fn parse_file_attribute(&mut self, tokenizer: &mut Tokenizer<'_>) -> Result<()> {
        let a = crate::parse::parse_attribute(tokenizer)?;
        self.unit.file_attributes.push(a);
        Ok(())
    }

    fn parse_class(&mut self, tokenizer: &mut Tokenizer<'_>) -> Result<()> {
        let c = crate::class::ClassParser::parse(tokenizer, self)?;
        self.unit.classes.push(c);
        Ok(())
    }

    fn parse_class_ref(&mut self, tokenizer: &mut Tokenizer<'_>) -> Result<()> {
        let (id, _) = parse_user_id(tokenizer)?;
        self.unit
            .symbol_refs
            .classes
            .push(ClassName::from_bytes(self.alloc, &id));
        Ok(())
    }

    fn parse_const_ref(&mut self, tokenizer: &mut Tokenizer<'_>) -> Result<()> {
        let (id, _) = parse_user_id(tokenizer)?;
        self.unit
            .symbol_refs
            .constants
            .push(ConstName::from_bytes(self.alloc, &id));
        Ok(())
    }

    fn parse_constant(&mut self, tokenizer: &mut Tokenizer<'_>) -> Result<()> {
        let c = parse_hack_constant(tokenizer)?;
        self.unit.constants.push(c);
        Ok(())
    }

    fn parse_fatal(&mut self, tokenizer: &mut Tokenizer<'_>) -> Result<()> {
        let loc = self.get_cur_src_loc();

        let op = parse_fatal_op(tokenizer)?;

        let message = tokenizer.expect_any_string()?;
        let message = message.unescaped_string()?;
        let message = bstr::BString::from(message);

        self.unit.fatal = Some(Fatal { op, message, loc });
        Ok(())
    }

    fn parse_func_ref(&mut self, tokenizer: &mut Tokenizer<'_>) -> Result<()> {
        let (id, _) = parse_user_id(tokenizer)?;
        self.unit
            .symbol_refs
            .functions
            .push(FunctionName::from_bytes(self.alloc, &id));
        Ok(())
    }

    fn parse_include_ref(&mut self, tokenizer: &mut Tokenizer<'_>) -> Result<()> {
        let incref = if tokenizer.next_is_identifier("relative")? {
            let a = tokenizer
                .expect_any_string()?
                .unescaped_bump_str(self.alloc)?;
            IncludePath::SearchPathRelative(a)
        } else if tokenizer.next_is_identifier("rooted")? {
            let a = tokenizer
                .expect_any_string()?
                .unescaped_bump_str(self.alloc)?;
            let b = tokenizer
                .expect_any_string()?
                .unescaped_bump_str(self.alloc)?;
            IncludePath::IncludeRootRelative(a, b)
        } else if tokenizer.next_is_identifier("doc")? {
            let a = tokenizer
                .expect_any_string()?
                .unescaped_bump_str(self.alloc)?;
            IncludePath::DocRootRelative(a)
        } else {
            let a = tokenizer
                .expect_any_string()?
                .unescaped_bump_str(self.alloc)?;
            IncludePath::Absolute(a)
        };
        self.unit.symbol_refs.includes.push(incref);
        Ok(())
    }

    fn parse_function(&mut self, tokenizer: &mut Tokenizer<'_>) -> Result<()> {
        let f = crate::func::FunctionParser::parse(tokenizer, self, None)?;
        self.unit.functions.push(f);
        Ok(())
    }

    fn parse_method(&mut self, tokenizer: &mut Tokenizer<'_>) -> Result<()> {
        let (clsname, clsloc) = parse_user_id(tokenizer)?;
        let clsid = ClassId::from_bytes(&clsname, &self.unit.strings);
        tokenizer.expect_identifier("::")?;

        let mut cs = crate::func::ClassState::default();
        let f = crate::func::FunctionParser::parse(tokenizer, self, Some(&mut cs))?;

        let class = self.unit.classes.iter_mut().find(|c| c.name == clsid);
        if let Some(class) = class {
            let Function {
                attributes,
                attrs,
                coeffects,
                flags: _,
                name,
                func,
            } = f;

            let name = MethodId::new(name.id);

            class.methods.push(Method {
                attributes,
                attrs,
                coeffects,
                flags: cs.flags,
                func,
                name,
                visibility: cs.visibility,
            });
        } else {
            return Err(clsloc.bail(format!(
                "Class '{}' not defined",
                String::from_utf8_lossy(&clsname)
            )));
        }

        Ok(())
    }

    fn parse_src_loc(&mut self, tokenizer: &mut Tokenizer<'_>) -> Result<()> {
        let src_loc = parse_src_loc(tokenizer, None)?;
        self.src_loc = Some(src_loc);
        Ok(())
    }

    fn parse_module(&mut self, tokenizer: &mut Tokenizer<'_>) -> Result<()> {
        parse!(tokenizer, <name:parse_class_id> "[" <attributes:parse_attribute,*> "]");

        let doc_comment = parse_doc_comment(tokenizer, self.alloc)?;

        let src_loc = self.get_cur_src_loc();

        self.unit.modules.push(Module {
            attributes,
            name,
            src_loc,
            doc_comment,
        });
        Ok(())
    }

    fn parse_module_use(&mut self, tokenizer: &mut Tokenizer<'_>) -> Result<()> {
        let module_use = tokenizer
            .expect_any_string()?
            .unescaped_bump_str(self.alloc)?;
        self.unit.module_use = Some(module_use);
        Ok(())
    }

    fn parse_typedef(&mut self, tokenizer: &mut Tokenizer<'_>) -> Result<()> {
        parse!(tokenizer, <name:parse_class_id> ":" <type_info:parse_type_info> "="
               <attributes:parse_attributes("<")> <type_structure:parse_typed_value> <attrs:parse_attr>);

        let loc = self.get_cur_src_loc();

        self.unit.typedefs.push(Typedef {
            attributes,
            attrs,
            loc,
            name,
            type_info,
            type_structure,
        });
        Ok(())
    }
}
