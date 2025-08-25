// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use anyhow::Result;
use anyhow::bail;
use assemble_opcode_macro::assemble_imm_for_enum;
use ffi::Vector;
use hhbc::BytesId;
use hhbc::StringId;

use crate::assemble;
use crate::assemble::AdataMap;
use crate::assemble::DeclMap;
use crate::lexer::Lexer;
use crate::token::Token;

assemble_imm_for_enum!(
    hhbc::BareThisOp,
    [
        BareThisOp::NeverNull,
        BareThisOp::NoNotice,
        BareThisOp::Notice
    ]
);

assemble_imm_for_enum!(
    hhbc::ClassGetCMode,
    [
        ClassGetCMode::Normal,
        ClassGetCMode::ExplicitConversion,
        ClassGetCMode::UnsafeBackdoor,
    ]
);

assemble_imm_for_enum!(
    hhbc::CollectionType,
    [
        CollectionType::ImmMap,
        CollectionType::ImmSet,
        CollectionType::ImmVector,
        CollectionType::Map,
        CollectionType::Pair,
        CollectionType::Set,
        CollectionType::Vector,
    ]
);

assemble_imm_for_enum!(
    hhbc::FatalOp,
    [FatalOp::Parse, FatalOp::Runtime, FatalOp::RuntimeOmitFrame,]
);

assemble_imm_for_enum!(
    hhbc::IncDecOp,
    [
        IncDecOp::PostDec,
        IncDecOp::PostInc,
        IncDecOp::PreDec,
        IncDecOp::PreInc,
    ]
);

assemble_imm_for_enum!(
    hhbc::InitPropOp,
    [InitPropOp::NonStatic, InitPropOp::Static]
);

assemble_imm_for_enum!(
    hhbc::IsLogAsDynamicCallOp,
    [
        IsLogAsDynamicCallOp::DontLogAsDynamicCall,
        IsLogAsDynamicCallOp::LogAsDynamicCall,
    ]
);

assemble_imm_for_enum!(
    hhbc::IsTypeOp,
    [
        IsTypeOp::ArrLike,
        IsTypeOp::Bool,
        IsTypeOp::Class,
        IsTypeOp::ClsMeth,
        IsTypeOp::Dbl,
        IsTypeOp::Dict,
        IsTypeOp::Func,
        IsTypeOp::Int,
        IsTypeOp::Keyset,
        IsTypeOp::LegacyArrLike,
        IsTypeOp::Null,
        IsTypeOp::Obj,
        IsTypeOp::Res,
        IsTypeOp::Scalar,
        IsTypeOp::Str,
        IsTypeOp::Vec,
    ]
);

assemble_imm_for_enum!(
    hhbc::MOpMode,
    [
        MOpMode::Define,
        MOpMode::InOut,
        MOpMode::None,
        MOpMode::Unset,
        MOpMode::Warn,
    ]
);

assemble_imm_for_enum!(
    hhbc::ObjMethodOp,
    [ObjMethodOp::NullSafe, ObjMethodOp::NullThrows,]
);

assemble_imm_for_enum!(
    hhbc::OODeclExistsOp,
    [
        OODeclExistsOp::Class,
        OODeclExistsOp::Interface,
        OODeclExistsOp::Trait,
    ]
);

assemble_imm_for_enum!(
    hhbc::QueryMOp,
    [
        QueryMOp::CGet,
        QueryMOp::CGetQuiet,
        QueryMOp::InOut,
        QueryMOp::Isset,
    ]
);

assemble_imm_for_enum!(
    hhbc::ReadonlyOp,
    [
        ReadonlyOp::Any,
        ReadonlyOp::CheckMutROCOW,
        ReadonlyOp::CheckROCOW,
        ReadonlyOp::Mutable,
        ReadonlyOp::Readonly,
    ]
);

assemble_imm_for_enum!(
    hhbc::SetOpOp,
    [
        SetOpOp::AndEqual,
        SetOpOp::ConcatEqual,
        SetOpOp::DivEqual,
        SetOpOp::MinusEqual,
        SetOpOp::ModEqual,
        SetOpOp::MulEqual,
        SetOpOp::OrEqual,
        SetOpOp::PlusEqual,
        SetOpOp::PowEqual,
        SetOpOp::SlEqual,
        SetOpOp::SrEqual,
        SetOpOp::XorEqual,
    ]
);

assemble_imm_for_enum!(hhbc::SetRangeOp, [SetRangeOp::Forward, SetRangeOp::Reverse]);

assemble_imm_for_enum!(
    hhbc::SpecialClsRef,
    [
        SpecialClsRef::LateBoundCls,
        SpecialClsRef::ParentCls,
        SpecialClsRef::SelfCls,
    ]
);

assemble_imm_for_enum!(
    hhbc::TypeStructResolveOp,
    [
        TypeStructResolveOp::DontResolve,
        TypeStructResolveOp::Resolve,
    ]
);
assemble_imm_for_enum!(
    hhbc::TypeStructEnforceKind,
    [TypeStructEnforceKind::Deep, TypeStructEnforceKind::Shallow]
);

assemble_imm_for_enum!(
    hhbc::AsTypeStructExceptionKind,
    [
        AsTypeStructExceptionKind::Error,
        AsTypeStructExceptionKind::Typehint
    ]
);

assemble_imm_for_enum!(
    hhbc::ContCheckOp,
    [ContCheckOp::IgnoreStarted, ContCheckOp::CheckStarted,]
);

pub(crate) trait AssembleImm<T> {
    fn assemble_imm(&mut self, decl_map: &DeclMap, adata: &AdataMap) -> Result<T>;
}

impl AssembleImm<i64> for Lexer<'_> {
    fn assemble_imm(&mut self, _: &DeclMap, _: &AdataMap) -> Result<i64> {
        self.expect_and_get_number()
    }
}

impl AssembleImm<hhbc::TypedValue> for Lexer<'_> {
    /// Assemble an AdataId, then lookup and clone the referenced Adata TypeValue
    fn assemble_imm(&mut self, _: &DeclMap, adata: &AdataMap) -> Result<hhbc::TypedValue> {
        let adata_id = self.expect_with(Token::into_global)?;
        debug_assert!(adata_id[0] == b'@');
        let src_id = hhbc::AdataId::parse(std::str::from_utf8(&adata_id[1..])?)?;
        adata
            .lookup(src_id)
            .ok_or_else(|| anyhow::anyhow!("Unknown AdataId {src_id}"))
            .cloned()
    }
}

impl AssembleImm<hhbc::ClassName> for Lexer<'_> {
    fn assemble_imm(&mut self, _: &DeclMap, _: &AdataMap) -> Result<hhbc::ClassName> {
        Ok(hhbc::ClassName::new(
            assemble::assemble_unescaped_unquoted_intern_str(self)?,
        ))
    }
}

impl AssembleImm<hhbc::ConstName> for Lexer<'_> {
    fn assemble_imm(&mut self, _: &DeclMap, _: &AdataMap) -> Result<hhbc::ConstName> {
        Ok(hhbc::ConstName::new(
            assemble::assemble_unescaped_unquoted_intern_str(self)?,
        ))
    }
}

impl AssembleImm<hhbc::FCallArgs> for Lexer<'_> {
    fn assemble_imm(&mut self, _: &DeclMap, _: &AdataMap) -> Result<hhbc::FCallArgs> {
        // <(fcargflags)*> numargs numrets inouts readonly async_eager_target context
        let fcargflags = assemble::assemble_fcallargsflags(self)?;
        let num_args = self.expect_and_get_number()?;
        let num_rets = self.expect_and_get_number()?;
        let inouts = assemble::assemble_inouts_or_readonly(self)?;
        let readonly = assemble::assemble_inouts_or_readonly(self)?;
        let async_eager_target = assemble::assemble_async_eager_target(self)?;
        let context = assemble::assemble_fcall_context(self)?;
        let fcargs = hhbc::FCallArgs::new(
            fcargflags,
            num_rets,
            num_args,
            inouts,
            readonly,
            async_eager_target,
            None,
        );
        Ok(hhbc::FCallArgs { context, ..fcargs })
    }
}

impl AssembleImm<hhbc::FloatBits> for Lexer<'_> {
    fn assemble_imm(&mut self, _: &DeclMap, _: &AdataMap) -> Result<hhbc::FloatBits> {
        Ok(hhbc::FloatBits(self.expect_and_get_number()?))
    }
}

impl AssembleImm<hhbc::FunctionName> for Lexer<'_> {
    fn assemble_imm(&mut self, _: &DeclMap, _: &AdataMap) -> Result<hhbc::FunctionName> {
        Ok(hhbc::FunctionName::new(
            assemble::assemble_unescaped_unquoted_intern_str(self)?,
        ))
    }
}

impl AssembleImm<hhbc::IterArgs> for Lexer<'_> {
    fn assemble_imm(&mut self, _: &DeclMap, _: &AdataMap) -> Result<hhbc::IterArgs> {
        // IterArgs { iter_id: IterId (~u32), flags: IterArgsFlags }
        // Ex: <BaseConst> 0
        let flags = assemble::assemble_iterargsflags(self)?;
        let idx: usize = self.expect_and_get_number()?;
        let iter_id = hhbc::IterId::new(idx);
        Ok(hhbc::IterArgs { iter_id, flags })
    }
}

impl AssembleImm<hhbc::IterId> for Lexer<'_> {
    fn assemble_imm(&mut self, _: &DeclMap, _: &AdataMap) -> Result<hhbc::IterId> {
        Ok(hhbc::IterId::new(self.expect_and_get_number()?))
    }
}

impl AssembleImm<hhbc::Label> for Lexer<'_> {
    fn assemble_imm(&mut self, _: &DeclMap, _: &AdataMap) -> Result<hhbc::Label> {
        assemble::assemble_label(self)
    }
}

impl AssembleImm<hhbc::Local> for Lexer<'_> {
    fn assemble_imm(&mut self, decl_map: &DeclMap, _: &AdataMap) -> Result<hhbc::Local> {
        // Returns the local (u32 idx) a var or unnamed corresponds to.
        // This information is based on the position of the var in parameters of a function/.declvars
        // or, if an unnamed, just the idx referenced (_1 -> idx 1)
        // $a -> idx where $a is stored in hcu body
        // _3 -> 3
        match self.next() {
            Some(Token::Variable(v, p)) => {
                let v = hhbc::intern(std::str::from_utf8(v)?);
                if let Some(local) = decl_map.get(&v) {
                    Ok(*local)
                } else {
                    bail!("Unknown local var: {:?} at {:?}", v, p);
                }
            }
            Some(Token::Identifier(i, _)) => {
                debug_assert!(i[0] == b'_');
                Ok(hhbc::Local::new(
                    std::str::from_utf8(&i[1..i.len()])?.parse()?,
                ))
            }
            Some(tok) => Err(tok.error("Unknown local")),
            None => Err(self.error("Expected local")),
        }
    }
}

impl AssembleImm<hhbc::LocalRange> for Lexer<'_> {
    fn assemble_imm(&mut self, _: &DeclMap, _: &AdataMap) -> Result<hhbc::LocalRange> {
        self.expect_str(Token::is_identifier, "L")?;
        self.expect(Token::is_colon)?;
        let start = hhbc::Local::new(self.expect_and_get_number()?);
        //self.expect(Token::is_plus)?; // Not sure if this exists yet
        let len = self.expect_and_get_number()?;
        Ok(hhbc::LocalRange { start, len })
    }
}

impl AssembleImm<hhbc::MemberKey> for Lexer<'_> {
    fn assemble_imm(&mut self, decl_map: &DeclMap, adata: &AdataMap) -> Result<hhbc::MemberKey> {
        // EC: stackIndex readOnlyOp | EL: local readOnlyOp | ET: string readOnlyOp | EI: int readOnlyOp
        // PC: stackIndex readOnlyOp | PL: local readOnlyOp | PT: propName readOnlyOp | QT: propName readOnlyOp
        let tok = self.expect_token()?;
        match tok.into_identifier()? {
            b"EC" => {
                self.expect(Token::is_colon)?;
                Ok(hhbc::MemberKey::EC(
                    self.assemble_imm(decl_map, adata)?,
                    self.assemble_imm(decl_map, adata)?,
                ))
            }
            b"EL" => {
                self.expect(Token::is_colon)?;
                Ok(hhbc::MemberKey::EL(
                    self.assemble_imm(decl_map, adata)?,
                    self.assemble_imm(decl_map, adata)?,
                ))
            }
            b"ET" => {
                self.expect(Token::is_colon)?;
                Ok(hhbc::MemberKey::ET(
                    hhbc::intern_bytes(escaper::unescape_literal_bytes_into_vec_bytes(
                        // In bp, print_quoted_str also escapes the string
                        escaper::unquote_slice(self.expect_with(Token::into_str_literal)?),
                    )?),
                    self.assemble_imm(decl_map, adata)?,
                ))
            }
            b"EI" => {
                self.expect(Token::is_colon)?;
                Ok(hhbc::MemberKey::EI(
                    self.expect_and_get_number()?,
                    self.assemble_imm(decl_map, adata)?,
                ))
            }
            b"PC" => {
                self.expect(Token::is_colon)?;
                Ok(hhbc::MemberKey::PC(
                    self.assemble_imm(decl_map, adata)?,
                    self.assemble_imm(decl_map, adata)?,
                ))
            }
            b"PL" => {
                self.expect(Token::is_colon)?;
                Ok(hhbc::MemberKey::PL(
                    self.assemble_imm(decl_map, adata)?,
                    self.assemble_imm(decl_map, adata)?,
                ))
            }
            b"PT" => {
                self.expect(Token::is_colon)?;
                Ok(hhbc::MemberKey::PT(
                    assemble::assemble_prop_name_from_str(self)?,
                    self.assemble_imm(decl_map, adata)?,
                ))
            }
            b"QT" => {
                self.expect(Token::is_colon)?;
                Ok(hhbc::MemberKey::QT(
                    assemble::assemble_prop_name_from_str(self)?,
                    self.assemble_imm(decl_map, adata)?,
                ))
            }
            b"W" => Ok(hhbc::MemberKey::W),
            _ => Err(tok.error("Expected a MemberKey")),
        }
    }
}

impl AssembleImm<hhbc::MethodName> for Lexer<'_> {
    fn assemble_imm(&mut self, _: &DeclMap, _: &AdataMap) -> Result<hhbc::MethodName> {
        Ok(hhbc::MethodName::new(
            assemble::assemble_unescaped_unquoted_intern_str(self)?,
        ))
    }
}

impl AssembleImm<hhbc::PropName> for Lexer<'_> {
    fn assemble_imm(&mut self, _: &DeclMap, _: &AdataMap) -> Result<hhbc::PropName> {
        Ok(hhbc::PropName::new(
            assemble::assemble_unescaped_unquoted_intern_str(self)?,
        ))
    }
}

impl AssembleImm<Vector<hhbc::Label>> for Lexer<'_> {
    fn assemble_imm(&mut self, _: &DeclMap, _: &AdataMap) -> Result<Vector<hhbc::Label>> {
        let mut labels = Vec::new();
        self.expect(Token::is_lt)?;
        while !self.peek_is(Token::is_gt) {
            labels.push(assemble::assemble_label(self)?)
        }
        self.expect(Token::is_gt)?;
        Ok(labels.into())
    }
}

impl AssembleImm<Vector<BytesId>> for Lexer<'_> {
    fn assemble_imm(&mut self, _: &DeclMap, _: &AdataMap) -> Result<Vector<BytesId>> {
        self.expect(Token::is_lt)?;
        let mut d = Vec::new();
        while !self.peek_is(Token::is_gt) {
            d.push(assemble::assemble_unescaped_unquoted_intern_bytes(self)?);
        }
        self.expect(Token::is_gt)?;
        Ok(d.into())
    }
}

impl AssembleImm<hhbc::StackIndex> for Lexer<'_> {
    fn assemble_imm(&mut self, _: &DeclMap, _: &AdataMap) -> Result<hhbc::StackIndex> {
        // StackIndex : u32
        self.expect_and_get_number()
    }
}

impl AssembleImm<StringId> for Lexer<'_> {
    fn assemble_imm(&mut self, _: &DeclMap, _: &AdataMap) -> Result<StringId> {
        assemble::assemble_unescaped_unquoted_intern_str(self)
    }
}

impl AssembleImm<BytesId> for Lexer<'_> {
    fn assemble_imm(&mut self, _: &DeclMap, _: &AdataMap) -> Result<BytesId> {
        assemble::assemble_unescaped_unquoted_intern_bytes(self)
    }
}

impl AssembleImm<hhbc::SwitchKind> for Lexer<'_> {
    fn assemble_imm(&mut self, _: &DeclMap, _: &AdataMap) -> Result<hhbc::SwitchKind> {
        let tok = self.expect_token()?;
        match tok.into_identifier()? {
            b"Unbounded" => Ok(hhbc::SwitchKind::Unbounded),
            b"Bounded" => Ok(hhbc::SwitchKind::Bounded),
            _ => Err(tok.error("Unknown switch kind")),
        }
    }
}
