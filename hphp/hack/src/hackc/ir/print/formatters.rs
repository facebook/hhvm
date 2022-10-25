// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

//! Formatting Functions
//!
//! These functions are used to print the IR in a pseudo-assembly format.
//!
//! The functions in this file (as opposed to print.rs) print partial lines of
//! IR. As a result they mostly will be in the form:
//! ```
//! struct FmtThing { .. }
//! impl Display for FmtThing { .. }
//! ```
//!

use std::fmt::Display;
use std::fmt::Formatter;
use std::fmt::Result;

use ffi::Str;
use ir_core::func::SrcLoc;
use ir_core::instr::BareThisOp;
use ir_core::instr::CollectionType;
use ir_core::instr::HasLoc;
use ir_core::instr::IncDecOp;
use ir_core::instr::InitPropOp;
use ir_core::instr::IsTypeOp;
use ir_core::instr::MOpMode;
use ir_core::instr::ReadonlyOp;
use ir_core::instr::SetOpOp;
use ir_core::instr::Special;
use ir_core::instr::SpecialClsRef;
use ir_core::string_intern::StringInterner;
use ir_core::*;

use crate::print::FuncContext;
use crate::util::FmtEscapedString;
use crate::util::FmtSep;

pub(crate) struct FmtAttr(pub Attr);

impl Display for FmtAttr {
    fn fmt(&self, w: &mut Formatter<'_>) -> Result {
        let FmtAttr(attr) = *self;
        FmtSep::new(
            "[",
            ", ",
            "]",
            [
                attr.is_final().then_some("is_final"),
                attr.is_sealed().then_some("is_sealed"),
                attr.is_abstract().then_some("is_abstract"),
                attr.is_interface().then_some("is_interface"),
                attr.is_trait().then_some("is_trait"),
                attr.is_const().then_some("is_const"),
                attr.no_dynamic_props().then_some("no_dynamic_props"),
                attr.needs_no_reifiedinit()
                    .then_some("needs_no_reifiedinit"),
                attr.is_late_init().then_some("is_late_init"),
                attr.is_no_bad_redeclare().then_some("is_no_bad_redeclare"),
                attr.initial_satisfies_tc()
                    .then_some("initial_satisfies_tc"),
                attr.no_implicit_null().then_some("no_implicit_null"),
                attr.has_system_initial().then_some("has_system_initial"),
                attr.is_deep_init().then_some("is_deep_init"),
                attr.is_lsb().then_some("is_lsb"),
                attr.is_static().then_some("is_static"),
                attr.is_readonly().then_some("is_readonly"),
                attr.is_no_injection().then_some("is_no_injection"),
                attr.is_interceptable().then_some("is_interceptable"),
                attr.is_empty().then_some("is_empty"),
            ]
            .into_iter()
            .flatten(),
            |w, s| w.write_str(s),
        )
        .fmt(w)
    }
}

pub(crate) struct FmtAttribute<'a>(pub &'a Attribute<'a>, pub &'a StringInterner);

impl Display for FmtAttribute<'_> {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
        let FmtAttribute(attr, strings) = *self;
        FmtIdentifier(attr.name.as_ref()).fmt(f)?;
        if !attr.arguments.is_empty() {
            write!(
                f,
                "({})",
                FmtSep::comma(attr.arguments.iter(), |w, tv| {
                    FmtTypedValue(tv, strings).fmt(w)
                })
            )?;
        }
        Ok(())
    }
}

pub(crate) struct FmtBareThisOp(pub BareThisOp);

impl Display for FmtBareThisOp {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
        let s = match self.0 {
            BareThisOp::Notice => "notice",
            BareThisOp::NoNotice => "no_notice",
            BareThisOp::NeverNull => "never_null",
            _ => panic!("bad BareThisOp value"),
        };
        f.write_str(s)
    }
}

pub struct FmtBid<'a>(pub &'a Func<'a>, pub BlockId, /* verbose */ pub bool);

impl Display for FmtBid<'_> {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
        let FmtBid(body, bid, verbose) = *self;
        if let Some(block) = body.get_block(bid) {
            let pname = block.pname_hint.as_ref();
            if !verbose {
                if let Some(pname) = pname {
                    return write!(f, "b{}[\"{}\"]", bid.as_usize(), pname);
                }
            }
        }

        FmtRawBid(bid).fmt(f)
    }
}

pub(crate) struct FmtCollectionType(pub(crate) CollectionType);

impl Display for FmtCollectionType {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
        let kind = match self.0 {
            CollectionType::Vector => "vector",
            CollectionType::Map => "map",
            CollectionType::Set => "set",
            CollectionType::Pair => "pair",
            CollectionType::ImmVector => "imm_vector",
            CollectionType::ImmMap => "imm_map",
            CollectionType::ImmSet => "imm_set",
            _ => panic!("bad CollectionType value"),
        };
        f.write_str(kind)
    }
}

pub(crate) struct FmtSetOpOp(pub SetOpOp);

impl FmtSetOpOp {
    pub(crate) fn as_str(&self) -> &'static str {
        match self.0 {
            SetOpOp::AndEqual => "&=",
            SetOpOp::ConcatEqual => ".=",
            SetOpOp::DivEqual => "/=",
            SetOpOp::MinusEqual => "-=",
            SetOpOp::ModEqual => "%=",
            SetOpOp::MulEqual => "*=",
            SetOpOp::OrEqual => "|=",
            SetOpOp::PlusEqual => "+=",
            SetOpOp::PowEqual => "**=",
            SetOpOp::SlEqual => "<<=",
            SetOpOp::SrEqual => ">>=",
            SetOpOp::XorEqual => "^=",
            _ => panic!("invalid SetOpOp value"),
        }
    }
}

impl Display for FmtSetOpOp {
    fn fmt(&self, w: &mut Formatter<'_>) -> Result {
        w.write_str(self.as_str())
    }
}

pub(crate) struct FmtFuncParams<'a>(pub &'a Func<'a>, pub &'a StringInterner);

impl Display for FmtFuncParams<'_> {
    fn fmt(&self, w: &mut Formatter<'_>) -> Result {
        let FmtFuncParams(func, strings) = *self;
        write!(
            w,
            "({})",
            FmtSep::comma(&func.params, |w, param| crate::print::print_param(
                w, strings, func, param
            ))
        )
    }
}

pub(crate) struct FmtIdentifier<'a>(pub(crate) &'a [u8]);

impl Display for FmtIdentifier<'_> {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
        // Ideally we print identifiers as non-quoted strings - but if they
        // contain "strange" characters then we need to quote and escape them.
        let needs_quote = !self.0.iter().all(u8::is_ascii_graphic);
        if needs_quote {
            let mut s = String::new();
            s.reserve(self.0.len() * 2);
            for &c in self.0 {
                if let Some(c) = char::from_u32(c as u32) {
                    if c.is_ascii_graphic() {
                        s.push(c);
                        continue;
                    }
                }
                s.push('\\');
                s.push('x');
                s.push(char::from_digit((c >> 4) as u32, 16).unwrap());
                s.push(char::from_digit((c & 15) as u32, 16).unwrap());
            }
            write!(f, "\"{}\"", s)
        } else {
            write!(f, "{}", std::str::from_utf8(self.0).unwrap())
        }
    }
}

pub(crate) struct FmtIdentifierId<'a>(pub UnitBytesId, pub &'a StringInterner);

impl Display for FmtIdentifierId<'_> {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
        let FmtIdentifierId(id, strings) = *self;
        let id = strings.lookup_bytes(id);
        FmtIdentifier(id).fmt(f)
    }
}

pub(crate) struct FmtConstant<'a, 'b>(pub(crate) &'b Constant<'a>, pub &'b StringInterner);

impl Display for FmtConstant<'_, '_> {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
        let FmtConstant(constant, strings) = self;
        match constant {
            Constant::Array(tv) => write!(f, "array({})", FmtTypedValue(tv, strings)),
            Constant::Bool(b) => write!(f, "{b}"),
            Constant::Dir => write!(f, "dir"),
            Constant::Double(value) => write!(f, "{}", value.0),
            Constant::File => write!(f, "file"),
            Constant::FuncCred => write!(f, "func_cred"),
            Constant::Int(value) => write!(f, "{}", value),
            Constant::Method => write!(f, "method"),
            Constant::Named(name) => write!(f, "constant({})", FmtIdentifier(name.as_bytes())),
            Constant::NewCol(k) => write!(f, "new_col({:?})", k),
            Constant::Null => write!(f, "null"),
            Constant::String(value) => FmtQuotedStr(value).fmt(f),
            Constant::Uninit => write!(f, "uninit"),
        }
    }
}

pub struct FmtVid<'a>(
    pub &'a Func<'a>,
    pub ValueId,
    /* verbose */ pub bool,
    pub &'a StringInterner,
);

impl Display for FmtVid<'_> {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
        let FmtVid(body, vid, verbose, strings) = *self;
        match vid.full() {
            FullInstrId::Constant(cid) => {
                if verbose {
                    FmtRawVid(vid).fmt(f)
                } else {
                    FmtConstantId(body, cid, strings).fmt(f)
                }
            }
            FullInstrId::Instr(iid) => {
                if let Some(instr) = body.get_instr(iid) {
                    if matches!(instr, Instr::Special(Special::Tombstone)) {
                        write!(f, "tombstone(%{})", iid.as_usize())
                    } else {
                        write!(f, "%{}", iid.as_usize())
                    }
                } else if iid == InstrId::NONE {
                    write!(f, "%none")
                } else {
                    write!(f, "unknown(%{})", iid.as_usize())
                }
            }
            FullInstrId::None => f.write_str("none"),
        }
    }
}

pub(crate) struct FmtIncDecOp(pub IncDecOp);

impl FmtIncDecOp {
    pub(crate) fn as_str(&self) -> &'static str {
        match self.0 {
            IncDecOp::PreInc => "pre_inc",
            IncDecOp::PostInc => "post_inc",
            IncDecOp::PreDec => "pre_dec",
            IncDecOp::PostDec => "post_dec",
            _ => panic!("bad IncDecOp value"),
        }
    }
}

impl Display for FmtIncDecOp {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
        f.write_str(self.as_str())
    }
}

pub(crate) struct FmtInitPropOp(pub InitPropOp);

impl Display for FmtInitPropOp {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
        let s = match self.0 {
            InitPropOp::Static => "static",
            InitPropOp::NonStatic => "non_static",
            _ => panic!("bad InitPropOp value"),
        };
        f.write_str(s)
    }
}

pub struct FmtInstr<'a, 'b>(pub &'b Func<'a>, pub &'b StringInterner, pub InstrId);

impl Display for FmtInstr<'_, '_> {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
        let FmtInstr(func, strings, iid) = self;
        let instr = func.get_instr(*iid).unwrap();
        let mut ctx = FuncContext {
            cur_loc_id: instr.loc_id(),
            live_instrs: Default::default(),
            strings,
            verbose: true,
        };
        crate::print::print_instr(f, &mut ctx, func, *iid, instr)?;
        Ok(())
    }
}

pub(crate) struct FmtIsTypeOp(pub IsTypeOp);

impl Display for FmtIsTypeOp {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
        let s = match self.0 {
            IsTypeOp::Null => "null",
            IsTypeOp::Bool => "bool",
            IsTypeOp::Int => "int",
            IsTypeOp::Dbl => "double",
            IsTypeOp::Str => "string",
            IsTypeOp::Obj => "object",
            IsTypeOp::Res => "resource",
            IsTypeOp::Scalar => "scalar",
            IsTypeOp::Keyset => "keyset",
            IsTypeOp::Dict => "dict",
            IsTypeOp::Vec => "vec",
            IsTypeOp::ArrLike => "array",
            IsTypeOp::ClsMeth => "clsmeth",
            IsTypeOp::Func => "func",
            IsTypeOp::LegacyArrLike => "legacy_array",
            IsTypeOp::Class => "class",
            _ => panic!("bad IsTypeOp value"),
        };
        f.write_str(s)
    }
}

pub struct FmtLid<'a>(pub LocalId, pub &'a StringInterner);

impl Display for FmtLid<'_> {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
        let FmtLid(ref lid, strings) = *self;
        match *lid {
            LocalId::Unnamed(id) => write!(f, "@{}", id.as_usize()),
            LocalId::Named(var) => FmtIdentifierId(var, strings).fmt(f),
        }
    }
}

pub(crate) struct FmtLids<'a>(pub(crate) &'a [LocalId], pub &'a StringInterner);

impl Display for FmtLids<'_> {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
        let FmtLids(slice, strings) = *self;
        write!(
            f,
            "[{}]",
            FmtSep::comma(slice.iter(), |f, v| { FmtLid(*v, strings).fmt(f) })
        )
    }
}

pub(crate) struct FmtConstantId<'a>(
    pub(crate) &'a Func<'a>,
    pub(crate) ConstantId,
    pub(crate) &'a StringInterner,
);

impl Display for FmtConstantId<'_> {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
        let FmtConstantId(body, cid, strings) = *self;
        let constant = body.constant(cid);
        FmtConstant(constant, strings).fmt(f)
    }
}

pub struct FmtLoc<'a>(pub &'a SrcLoc);

impl Display for FmtLoc<'_> {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
        write!(
            f,
            "{}:{},{}:{}",
            self.0.line_begin, self.0.col_begin, self.0.line_end, self.0.col_end
        )
    }
}

pub struct FmtLocId<'a>(pub &'a Func<'a>, pub LocId);

impl Display for FmtLocId<'_> {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
        let FmtLocId(func, loc_id) = *self;
        if let Some(loc) = func.locs.get(loc_id) {
            FmtLoc(loc).fmt(f)
        } else {
            write!(f, "<unknown loc #{loc_id}>")
        }
    }
}

pub(crate) struct FmtMOpMode(pub MOpMode);

impl Display for FmtMOpMode {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
        let s = match self.0 {
            MOpMode::None => "",
            MOpMode::Warn => "warn",
            MOpMode::Define => "define",
            MOpMode::Unset => "unset",
            MOpMode::InOut => "inout",
            _ => panic!("bad MopMode value"),
        };
        f.write_str(s)
    }
}

pub(crate) struct FmtOptKeyValue<'a>(pub Option<LocalId>, pub LocalId, pub &'a StringInterner);

impl Display for FmtOptKeyValue<'_> {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
        let FmtOptKeyValue(key, value, strings) = *self;
        match key {
            Some(key) => write!(f, "{} => {}", FmtLid(key, strings), FmtLid(value, strings)),
            None => write!(f, "{}", FmtLid(value, strings)),
        }
    }
}

pub(crate) struct FmtQuotedStr<'a>(pub(crate) &'a Str<'a>);

impl Display for FmtQuotedStr<'_> {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
        FmtEscapedString(self.0.as_ref()).fmt(f)
    }
}

pub(crate) struct FmtQuotedStringId<'a>(pub(crate) UnitBytesId, pub(crate) &'a StringInterner);

impl Display for FmtQuotedStringId<'_> {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
        let FmtQuotedStringId(id, strings) = *self;
        if id == UnitBytesId::NONE {
            write!(f, "none")
        } else {
            let name = strings.lookup_bytes(id);
            FmtEscapedString(name).fmt(f)
        }
    }
}

pub struct FmtRawBid(pub BlockId);

impl Display for FmtRawBid {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
        let FmtRawBid(bid) = *self;
        if bid == BlockId::NONE {
            f.write_str("none")
        } else {
            write!(f, "b{}", bid.as_usize())
        }
    }
}

pub struct FmtRawVid(pub ValueId);

impl Display for FmtRawVid {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
        match self.0.full() {
            FullInstrId::Instr(iid) => {
                if iid == InstrId::NONE {
                    f.write_str("%none")
                } else {
                    write!(f, "%{}", iid.as_usize())
                }
            }
            FullInstrId::Constant(cid) => {
                if cid == ConstantId::NONE {
                    f.write_str("#none")
                } else {
                    write!(f, "#{}", cid.as_usize())
                }
            }
            FullInstrId::None => f.write_str("none"),
        }
    }
}

pub(crate) struct FmtReadonly(pub(crate) ReadonlyOp);

impl Display for FmtReadonly {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
        match self.0 {
            ReadonlyOp::Readonly => write!(f, "readonly"),
            ReadonlyOp::Mutable => write!(f, "mutable"),
            ReadonlyOp::Any => write!(f, "any"),
            ReadonlyOp::CheckROCOW => write!(f, "check_ro_cow"),
            ReadonlyOp::CheckMutROCOW => write!(f, "check_mut_ro_cow"),
            _ => panic!("bad ReadonlyOp value"),
        }
    }
}

pub(crate) struct FmtSpecialClsRef(pub(crate) SpecialClsRef);

impl Display for FmtSpecialClsRef {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
        let s = match self.0 {
            SpecialClsRef::LateBoundCls => "late_bound",
            SpecialClsRef::SelfCls => "self",
            SpecialClsRef::ParentCls => "parent",
            _ => panic!("bad SpecialClsRef value"),
        };
        f.write_str(s)
    }
}

pub(crate) struct FmtTParams<'a>(
    pub(crate) &'a ClassIdMap<TParamBounds>,
    pub &'a StringInterner,
);

impl Display for FmtTParams<'_> {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
        let FmtTParams(map, strings) = self;
        if map.is_empty() {
            Ok(())
        } else {
            write!(
                f,
                "<{}>",
                FmtSep::comma(map.iter(), |f, (name, bounds)| {
                    FmtIdentifierId(name.id, strings).fmt(f)?;
                    if !bounds.bounds.is_empty() {
                        write!(f, ": ")?;
                        let mut sep = "";
                        for bound in bounds.bounds.iter() {
                            write!(f, "{sep}{}", bound.display(strings))?;
                            sep = " + ";
                        }
                    }
                    Ok(())
                })
            )
        }
    }
}

pub(crate) struct FmtShadowedTParams<'a>(
    pub(crate) &'a Vec<ClassId>,
    pub(crate) &'a StringInterner,
);

impl Display for FmtShadowedTParams<'_> {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
        let FmtShadowedTParams(vec, strings) = *self;
        if vec.is_empty() {
            Ok(())
        } else {
            write!(
                f,
                "[{}]",
                FmtSep::comma(vec.iter(), |f, name| {
                    FmtIdentifierId(name.id, strings).fmt(f)
                })
            )
        }
    }
}

pub(crate) struct FmtTypedValue<'a>(pub &'a TypedValue, pub &'a StringInterner);

impl Display for FmtTypedValue<'_> {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
        let FmtTypedValue(tv, strings) = *self;
        match tv {
            TypedValue::Uninit => write!(f, "uninit"),
            TypedValue::Int(v) => {
                write!(f, "{}", v)
            }
            TypedValue::Bool(b) => f.write_str(if *b { "true" } else { "false" }),
            TypedValue::Float(v) => write!(f, "{}", v.to_f64()),
            TypedValue::String(v) => FmtEscapedString(strings.lookup_bytes(*v)).fmt(f),
            TypedValue::LazyClass(_) => todo!("unhandled: {:?}", self.0),
            TypedValue::Null => f.write_str("null"),
            TypedValue::Vec(values) => {
                write!(
                    f,
                    "vec[{}]",
                    FmtSep::comma(values.iter(), |f, v| { FmtTypedValue(v, strings).fmt(f) })
                )
            }
            TypedValue::Keyset(values) => {
                write!(
                    f,
                    "keyset[{}]",
                    FmtSep::comma(values.iter(), |f, v| { FmtArrayKey(v, strings).fmt(f) })
                )
            }
            TypedValue::Dict(values) => {
                write!(
                    f,
                    "dict[{}]",
                    FmtSep::comma(values.iter(), |f, (key, value)| {
                        write!(
                            f,
                            "{} => {}",
                            FmtArrayKey(key, strings),
                            FmtTypedValue(value, strings)
                        )
                    })
                )
            }
        }
    }
}

pub(crate) struct FmtArrayKey<'a>(pub &'a ArrayKey, pub &'a StringInterner);

impl Display for FmtArrayKey<'_> {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
        let FmtArrayKey(tv, strings) = *self;
        match tv {
            ArrayKey::Int(v) => {
                write!(f, "{}", v)
            }
            ArrayKey::String(v) => FmtEscapedString(strings.lookup_bytes(*v)).fmt(f),
            ArrayKey::LazyClass(v) => FmtEscapedString(strings.lookup_bytes(*v)).fmt(f),
        }
    }
}

pub(crate) struct FmtVisibility(pub Visibility);

impl Display for FmtVisibility {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
        let vis = match self.0 {
            Visibility::Private => "private",
            Visibility::Public => "public",
            Visibility::Protected => "protected",
            Visibility::Internal => "internal",
        };
        vis.fmt(f)
    }
}
