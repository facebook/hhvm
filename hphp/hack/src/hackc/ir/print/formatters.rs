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

use ffi::Pair;
use ffi::Str;
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
                attr.is_final().then(|| "is_final"),
                attr.is_sealed().then(|| "is_sealed"),
                attr.is_abstract().then(|| "is_abstract"),
                attr.is_interface().then(|| "is_interface"),
                attr.is_trait().then(|| "is_trait"),
                attr.is_const().then(|| "is_const"),
                attr.no_dynamic_props().then(|| "no_dynamic_props"),
                attr.needs_no_reifiedinit().then(|| "needs_no_reifiedinit"),
                attr.is_late_init().then(|| "is_late_init"),
                attr.is_no_bad_redeclare().then(|| "is_no_bad_redeclare"),
                attr.initial_satisfies_tc().then(|| "initial_satisfies_tc"),
                attr.no_implicit_null().then(|| "no_implicit_null"),
                attr.has_system_initial().then(|| "has_system_initial"),
                attr.is_deep_init().then(|| "is_deep_init"),
                attr.is_lsb().then(|| "is_lsb"),
                attr.is_static().then(|| "is_static"),
                attr.is_readonly().then(|| "is_readonly"),
                attr.is_no_injection().then(|| "is_no_injection"),
                attr.is_interceptable().then(|| "is_interceptable"),
                attr.is_empty().then(|| "is_empty"),
            ]
            .into_iter()
            .flatten(),
            |w, s| w.write_str(s),
        )
        .fmt(w)
    }
}

pub(crate) struct FmtAttribute<'a>(pub &'a Attribute<'a>);

impl Display for FmtAttribute<'_> {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
        let FmtAttribute(attr) = self;
        FmtIdentifier(attr.name.as_ref()).fmt(f)?;
        if !attr.arguments.is_empty() {
            write!(
                f,
                "({})",
                FmtSep::comma(attr.arguments.iter(), |w, tv| { FmtTypedValue(tv).fmt(w) })
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
            SetOpOp::MinusEqualO => "-=o",
            SetOpOp::ModEqual => "%=",
            SetOpOp::MulEqual => "*=",
            SetOpOp::MulEqualO => "*=o",
            SetOpOp::OrEqual => "|=",
            SetOpOp::PlusEqual => "+=",
            SetOpOp::PlusEqualO => "+=o",
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

pub(crate) struct FmtFuncParams<'a>(pub &'a Func<'a>, pub &'a StringInterner<'a>);

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

pub(crate) struct FmtIdentifierId<'a, 'b>(pub UnitStringId, pub &'b StringInterner<'a>);

impl Display for FmtIdentifierId<'_, '_> {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
        let FmtIdentifierId(id, strings) = *self;
        let id = strings.lookup(id).as_bytes();
        FmtIdentifier(id).fmt(f)
    }
}

pub(crate) struct FmtLiteral<'a, 'b>(pub(crate) &'b Literal<'a>);

impl Display for FmtLiteral<'_, '_> {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
        let FmtLiteral(literal) = self;
        match literal {
            Literal::Bool(false) => write!(f, "false")?,
            Literal::Bool(true) => write!(f, "true")?,
            Literal::Dict(name) => write!(f, "dict({})", FmtQuotedStr(name))?,
            Literal::Dir => write!(f, "dir")?,
            Literal::Double(value) => write!(f, "{}", value.0)?,
            Literal::File => write!(f, "file")?,
            Literal::FuncCred => write!(f, "func_cred")?,
            Literal::Int(value) => write!(f, "{}", value)?,
            Literal::Keyset(name) => write!(f, "keyset({})", FmtQuotedStr(name))?,
            Literal::Method => write!(f, "method")?,
            Literal::Named(name) => write!(f, "literal({})", FmtIdentifier(name.as_bytes()))?,
            Literal::NewCol(k) => write!(f, "new_col({:?})", k)?,
            Literal::Null => write!(f, "null")?,
            Literal::String(value) => FmtQuotedStr(value).fmt(f)?,
            Literal::Uninit => write!(f, "uninit")?,
            Literal::Vec(name) => write!(f, "vec({})", FmtQuotedStr(name))?,
        }

        Ok(())
    }
}

pub struct FmtVid<'a>(pub &'a Func<'a>, pub ValueId, /* verbose */ pub bool);

impl Display for FmtVid<'_> {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
        let FmtVid(body, vid, verbose) = *self;
        match vid.full() {
            FullInstrId::Literal(lid) => {
                if verbose {
                    FmtRawVid(vid).fmt(f)
                } else {
                    FmtLiteralId(body, lid).fmt(f)
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
            IncDecOp::PreIncO => "pre_inc_o",
            IncDecOp::PostIncO => "post_inc_o",
            IncDecOp::PreDecO => "pre_dec_o",
            IncDecOp::PostDecO => "post_dec_o",
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

pub struct FmtInstr<'a, 'b>(pub &'b Func<'a>, pub &'b StringInterner<'a>, pub InstrId);

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

pub struct FmtLid<'a, 'b>(pub LocalId, pub &'b StringInterner<'a>);

impl Display for FmtLid<'_, '_> {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
        let FmtLid(ref lid, strings) = *self;
        match *lid {
            LocalId::Unnamed(id) => write!(f, "@{}", id.as_usize()),
            LocalId::Named(var) => FmtIdentifierId(var, strings).fmt(f),
        }
    }
}

pub(crate) struct FmtLids<'a, 'b>(pub(crate) &'b [LocalId], pub &'b StringInterner<'a>);

impl Display for FmtLids<'_, '_> {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
        let FmtLids(slice, strings) = *self;
        write!(
            f,
            "[{}]",
            FmtSep::comma(slice.iter(), |f, v| { FmtLid(*v, strings).fmt(f) })
        )
    }
}

pub(crate) struct FmtLiteralId<'a>(pub(crate) &'a Func<'a>, pub(crate) LiteralId);

impl Display for FmtLiteralId<'_> {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
        let FmtLiteralId(body, lid) = *self;
        let literal = body.literal(lid);
        FmtLiteral(literal).fmt(f)
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

pub(crate) struct FmtOptKeyValue<'a, 'b>(
    pub Option<LocalId>,
    pub LocalId,
    pub &'b StringInterner<'a>,
);

impl Display for FmtOptKeyValue<'_, '_> {
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

pub(crate) struct FmtQuotedStringId<'a, 'b>(
    pub(crate) UnitStringId,
    pub(crate) &'b StringInterner<'a>,
);

impl Display for FmtQuotedStringId<'_, '_> {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
        let FmtQuotedStringId(id, strings) = *self;
        if id == UnitStringId::NONE {
            write!(f, "none")
        } else {
            let name = strings.lookup(id);
            FmtEscapedString(name.as_bytes()).fmt(f)
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
            FullInstrId::Literal(lid) => {
                if lid == LiteralId::NONE {
                    f.write_str("#none")
                } else {
                    write!(f, "#{}", lid.as_usize())
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
    pub(crate) &'a ClassIdMap<TParamBounds<'a>>,
    pub &'a StringInterner<'a>,
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
                            write!(f, "{sep}{}", FmtType(bound))?;
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
    pub(crate) &'a StringInterner<'a>,
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

pub(crate) struct FmtType<'a>(pub(crate) &'a Type<'a>);

impl Display for FmtType<'_> {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
        match self.0 {
            Type::Empty => write!(f, "empty"),
            Type::Flags(flags, inner) => write!(f, "flags({}, {})", flags, FmtType(inner)),
            Type::None => write!(f, "unspecified"),
            Type::User(name) => write!(f, "{}", FmtIdentifier(name.as_ref())),
            Type::UserNoConstraint(name) => {
                write!(f, "unconstrained({})", FmtIdentifier(name.as_ref()))
            }
            Type::UserWithConstraint(name, constraint) => {
                write!(
                    f,
                    "constrained({}, {})",
                    FmtIdentifier(name.as_ref()),
                    FmtIdentifier(constraint.as_ref())
                )
            }
            Type::Void => write!(f, "void"),
        }
    }
}

pub(crate) struct FmtTypedValue<'a>(pub &'a TypedValue<'a>);

impl Display for FmtTypedValue<'_> {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result {
        match self.0 {
            TypedValue::Uninit => write!(f, "uninit"),
            TypedValue::Int(v) => {
                write!(f, "{}", v)
            }
            TypedValue::Bool(b) => f.write_str(if *b { "true" } else { "false" }),
            TypedValue::Float(v) => write!(f, "{}", v.to_f64()),
            TypedValue::String(v) => FmtEscapedString(v.as_ref()).fmt(f),
            TypedValue::LazyClass(_) => todo!("unhandled: {:?}", self.0),
            TypedValue::Null => f.write_str("null"),
            TypedValue::Vec(values) => {
                write!(
                    f,
                    "vec[{}]",
                    FmtSep::comma(values.as_ref(), |f, v| { FmtTypedValue(v).fmt(f) })
                )
            }
            TypedValue::Keyset(values) => {
                write!(
                    f,
                    "keyset[{}]",
                    FmtSep::comma(values.as_ref(), |f, v| { FmtTypedValue(v).fmt(f) })
                )
            }
            TypedValue::Dict(values) => {
                write!(
                    f,
                    "dict[{}]",
                    FmtSep::comma(values.as_ref(), |f, Pair(k, v)| {
                        write!(f, "{} => {}", FmtTypedValue(k), FmtTypedValue(v))
                    })
                )
            }
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
