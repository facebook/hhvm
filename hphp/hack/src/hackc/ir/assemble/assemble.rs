// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::io::Read;
use std::path::Path;

use anyhow::Context;
use anyhow::Result;
use ir_core::ClassName;
use ir_core::ConstName;
use ir_core::Fatal;
use ir_core::Function;
use ir_core::FunctionName;
use ir_core::IncludePath;
use ir_core::Method;
use ir_core::MethodName;
use ir_core::Module;
use ir_core::ModuleName;
use ir_core::SrcLoc;
use ir_core::Typedef;
use ir_core::Unit;
use parse_macro_ir::parse;

use crate::parse::parse_attr;
use crate::parse::parse_attribute;
use crate::parse::parse_attributes;
use crate::parse::parse_class_name;
use crate::parse::parse_comma_list;
use crate::parse::parse_doc_comment;
use crate::parse::parse_fatal_op;
use crate::parse::parse_hack_constant;
use crate::parse::parse_module_name;
use crate::parse::parse_src_loc;
use crate::parse::parse_type_info;
use crate::parse::parse_typed_value;
use crate::parse::parse_user_id;
use crate::tokenizer::Tokenizer;

pub fn unit_from_path(path: &Path) -> Result<Unit> {
    use std::fs::File;
    let mut file = File::open(path)?;
    read_unit(&mut file, &format!("{}", path.display()))
}

pub fn unit_from_string(input: &str) -> Result<Unit> {
    let mut input = input.as_bytes();
    read_unit(&mut input, "<string>")
}

pub fn read_unit(read: &mut dyn Read, filename: &str) -> Result<Unit> {
    let mut tokenizer = Tokenizer::new(read, filename);
    let unit = UnitParser::parse(&mut tokenizer)?;
    Ok(unit)
}

pub(crate) struct UnitParser {
    pub(crate) unit: Unit,
    pub(crate) src_loc: Option<SrcLoc>,
}

impl UnitParser {
    pub(crate) fn get_cur_src_loc(&self) -> SrcLoc {
        self.src_loc.as_ref().cloned().unwrap_or_default()
    }
}

impl UnitParser {
    fn parse(tokenizer: &mut Tokenizer<'_>) -> Result<Unit> {
        let mut state = UnitParser {
            unit: Unit {
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
            .push(ClassName::intern(std::str::from_utf8(&id)?));
        Ok(())
    }

    fn parse_const_ref(&mut self, tokenizer: &mut Tokenizer<'_>) -> Result<()> {
        let (id, _) = parse_user_id(tokenizer)?;
        self.unit
            .symbol_refs
            .constants
            .push(ConstName::intern(std::str::from_utf8(&id)?));
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
        let message = message.unescaped_string()?.into();

        self.unit.fatal = Some(Fatal { op, message, loc });
        Ok(())
    }

    fn parse_func_ref(&mut self, tokenizer: &mut Tokenizer<'_>) -> Result<()> {
        let (id, _) = parse_user_id(tokenizer)?;
        self.unit
            .symbol_refs
            .functions
            .push(FunctionName::intern(std::str::from_utf8(&id)?));
        Ok(())
    }

    fn parse_include_ref(&mut self, tokenizer: &mut Tokenizer<'_>) -> Result<()> {
        let incref = if tokenizer.next_is_identifier("relative")? {
            let a = ir_core::intern_bytes(tokenizer.expect_any_string()?.unescaped_string()?);
            IncludePath::SearchPathRelative(a)
        } else if tokenizer.next_is_identifier("rooted")? {
            let a = ir_core::intern_bytes(tokenizer.expect_any_string()?.unescaped_string()?);
            let b = ir_core::intern_bytes(tokenizer.expect_any_string()?.unescaped_string()?);
            IncludePath::IncludeRootRelative(a, b)
        } else if tokenizer.next_is_identifier("doc")? {
            let a = ir_core::intern_bytes(tokenizer.expect_any_string()?.unescaped_string()?);
            IncludePath::DocRootRelative(a)
        } else {
            let a = ir_core::intern_bytes(tokenizer.expect_any_string()?.unescaped_string()?);
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
        let clsid = ClassName::from_utf8(&clsname)?;
        tokenizer.expect_identifier("::")?;

        let mut cs = crate::func::ClassState::default();
        let f = crate::func::FunctionParser::parse(tokenizer, self, Some(&mut cs))?;

        let class = self.unit.classes.iter_mut().find(|c| c.name == clsid);
        if let Some(class) = class {
            let Function {
                flags: _,
                name,
                func,
            } = f;

            class.methods.push(Method {
                flags: cs.flags,
                func,
                name: MethodName::new(name.as_string_id()),
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
        let src_loc = parse_src_loc(tokenizer)?;
        self.src_loc = Some(src_loc);
        Ok(())
    }

    fn parse_module(&mut self, tokenizer: &mut Tokenizer<'_>) -> Result<()> {
        parse!(tokenizer, <name:parse_module_name> "[" <attributes:parse_attribute,*> "]");

        let doc_comment = parse_doc_comment(tokenizer)?;

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
        let module_use =
            ModuleName::from_utf8(&tokenizer.expect_any_string()?.unescaped_string()?)?;
        self.unit.module_use = Some(module_use);
        Ok(())
    }

    fn parse_typedef(&mut self, tokenizer: &mut Tokenizer<'_>) -> Result<()> {
        parse!(tokenizer, <vis:parse_user_id> <name:parse_class_name> ":" <type_info_union:parse_type_info,*> "="
               <attributes:parse_attributes("<")> <type_structure:parse_typed_value> <attrs:parse_attr>);

        let loc = self.get_cur_src_loc();

        self.unit.typedefs.push(Typedef {
            attributes,
            attrs,
            loc,
            name,
            type_info_union,
            type_structure,
            case_type: &vis.0 == b"case_type",
        });
        Ok(())
    }
}
