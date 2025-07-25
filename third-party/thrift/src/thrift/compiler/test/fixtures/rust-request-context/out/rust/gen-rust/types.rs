// @generated by Thrift for thrift/compiler/test/fixtures/rust-request-context/src/module.thrift
// This file is probably not the place you want to edit!


#![recursion_limit = "100000000"]
#![allow(non_camel_case_types, non_snake_case, non_upper_case_globals, unused_crate_dependencies, clippy::redundant_closure, clippy::type_complexity)]

pub mod services;

#[allow(unused_imports)]
pub(crate) use crate as types;

#[derive(Clone, PartialEq)]
pub struct MyStruct {
    pub MyIntField: ::std::primitive::i64,
    pub MyStringField: ::std::string::String,
    pub MyDataField: crate::types::MyDataItem,
    pub myEnum: crate::types::MyEnum,
    // This field forces `..Default::default()` when instantiating this
    // struct, to make code future-proof against new fields added later to
    // the definition in Thrift. If you don't want this, add the annotation
    // `@rust.Exhaustive` to the Thrift struct to eliminate this field.
    #[doc(hidden)]
    pub _dot_dot_Default_default: self::dot_dot::OtherFields,
}

#[derive(Clone, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct MyDataItem {
    // This field forces `..Default::default()` when instantiating this
    // struct, to make code future-proof against new fields added later to
    // the definition in Thrift. If you don't want this, add the annotation
    // `@rust.Exhaustive` to the Thrift struct to eliminate this field.
    #[doc(hidden)]
    pub _dot_dot_Default_default: self::dot_dot::OtherFields,
}

#[derive(Clone, PartialEq, Debug)]
pub enum MyUnion {
    myEnum(crate::types::MyEnum),
    myStruct(crate::types::MyStruct),
    myDataItem(crate::types::MyDataItem),
    UnknownField(::std::primitive::i32),
}

#[derive(Clone, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct MyException {
    // This field forces `..Default::default()` when instantiating this
    // struct, to make code future-proof against new fields added later to
    // the definition in Thrift. If you don't want this, add the annotation
    // `@rust.Exhaustive` to the Thrift struct to eliminate this field.
    #[doc(hidden)]
    pub _dot_dot_Default_default: self::dot_dot::OtherFields,
}

impl ::fbthrift::ExceptionInfo for MyException {
    fn exn_value(&self) -> String {
        format!("{:?}", self)
    }

    #[inline]
    fn exn_is_declared(&self) -> bool { true }
}

impl ::std::error::Error for MyException {}

impl ::std::fmt::Display for MyException {
    fn fmt(&self, f: &mut ::std::fmt::Formatter) -> ::std::fmt::Result {
        write!(f, "{:?}", self)
    }
}


#[derive(Copy, Clone, Eq, PartialEq, Ord, PartialOrd, Hash)]
pub struct MyEnum(pub ::std::primitive::i32);

impl MyEnum {
    pub const MyValue1: Self = MyEnum(0i32);
    pub const MyValue2: Self = MyEnum(1i32);
}

impl ::fbthrift::ThriftEnum for MyEnum {
    fn enumerate() -> &'static [(Self, &'static ::std::primitive::str)] {
        &[
            (Self::MyValue1, "MyValue1"),
            (Self::MyValue2, "MyValue2"),
        ]
    }

    fn variants() -> &'static [&'static ::std::primitive::str] {
        &[
            "MyValue1",
            "MyValue2",
        ]
    }

    fn variant_values() -> &'static [Self] {
        &[
            Self::MyValue1,
            Self::MyValue2,
        ]
    }

    fn inner_value(&self) -> i32 {
        self.0
    }
}

#[allow(clippy::derivable_impls)]
impl ::std::default::Default for MyEnum {
    fn default() -> Self {
        Self(0)
    }
}

impl<'a> ::std::convert::From<&'a MyEnum> for ::std::primitive::i32 {
    #[inline]
    fn from(x: &'a MyEnum) -> Self {
        x.0
    }
}

impl ::std::convert::From<MyEnum> for ::std::primitive::i32 {
    #[inline]
    fn from(x: MyEnum) -> Self {
        x.0
    }
}

impl ::std::convert::From<::std::primitive::i32> for MyEnum {
    #[inline]
    fn from(x: ::std::primitive::i32) -> Self {
        Self(x)
    }
}

impl ::std::fmt::Display for MyEnum {
    fn fmt(&self, fmt: &mut ::std::fmt::Formatter) -> ::std::fmt::Result {
        static VARIANTS_BY_NUMBER: &[(&::std::primitive::str, ::std::primitive::i32)] = &[
            ("MyValue1", 0),
            ("MyValue2", 1),
        ];
        ::fbthrift::help::enum_display(VARIANTS_BY_NUMBER, fmt, self.0)
    }
}

impl ::std::fmt::Debug for MyEnum {
    fn fmt(&self, fmt: &mut ::std::fmt::Formatter) -> ::std::fmt::Result {
        write!(fmt, "MyEnum::{}", self)
    }
}

impl ::std::str::FromStr for MyEnum {
    type Err = ::anyhow::Error;

    fn from_str(string: &::std::primitive::str) -> ::std::result::Result<Self, Self::Err> {
        static VARIANTS_BY_NAME: &[(&::std::primitive::str, ::std::primitive::i32)] = &[
            ("MyValue1", 0),
            ("MyValue2", 1),
        ];
        ::fbthrift::help::enum_from_str(VARIANTS_BY_NAME, string, "MyEnum").map(Self)
    }
}

impl ::fbthrift::GetTType for MyEnum {
    const TTYPE: ::fbthrift::TType = ::fbthrift::TType::I32;
}

impl<P> ::fbthrift::Serialize<P> for MyEnum
where
    P: ::fbthrift::ProtocolWriter,
{
    #[inline]
    fn rs_thrift_write(&self, p: &mut P) {
        p.write_i32(self.into())
    }
}

impl<P> ::fbthrift::Deserialize<P> for MyEnum
where
    P: ::fbthrift::ProtocolReader,
{
    #[inline]
    fn rs_thrift_read(p: &mut P) -> ::anyhow::Result<Self> {
        ::std::result::Result::Ok(Self::from(::anyhow::Context::context(p.read_i32(), "Expected a number indicating enum variant")?))
    }
}

#[allow(clippy::derivable_impls)]
impl ::std::default::Default for self::MyStruct {
    fn default() -> Self {
        Self {
            MyIntField: ::std::default::Default::default(),
            MyStringField: ::std::default::Default::default(),
            MyDataField: ::std::default::Default::default(),
            myEnum: ::std::default::Default::default(),
            _dot_dot_Default_default: self::dot_dot::OtherFields(()),
        }
    }
}

impl ::std::fmt::Debug for self::MyStruct {
    fn fmt(&self, formatter: &mut ::std::fmt::Formatter) -> ::std::fmt::Result {
        formatter
            .debug_struct("MyStruct")
            .field("MyIntField", &self.MyIntField)
            .field("MyStringField", &self.MyStringField)
            .field("MyDataField", &self.MyDataField)
            .field("myEnum", &self.myEnum)
            .finish()
    }
}

unsafe impl ::std::marker::Send for self::MyStruct {}
unsafe impl ::std::marker::Sync for self::MyStruct {}
impl ::std::marker::Unpin for self::MyStruct {}
impl ::std::panic::RefUnwindSafe for self::MyStruct {}
impl ::std::panic::UnwindSafe for self::MyStruct {}

impl ::fbthrift::GetTType for self::MyStruct {
    const TTYPE: ::fbthrift::TType = ::fbthrift::TType::Struct;
}

impl ::fbthrift::GetTypeNameType for self::MyStruct {
    fn type_name_type() -> fbthrift::TypeNameType {
        ::fbthrift::TypeNameType::StructType
    }
}

impl<P> ::fbthrift::Serialize<P> for self::MyStruct
where
    P: ::fbthrift::ProtocolWriter,
{
    #[inline]
    fn rs_thrift_write(&self, p: &mut P) {
        p.write_struct_begin("MyStruct");
        p.write_field_begin("MyIntField", ::fbthrift::TType::I64, 1);
        ::fbthrift::Serialize::rs_thrift_write(&self.MyIntField, p);
        p.write_field_end();
        p.write_field_begin("MyStringField", ::fbthrift::TType::String, 2);
        ::fbthrift::Serialize::rs_thrift_write(&self.MyStringField, p);
        p.write_field_end();
        p.write_field_begin("MyDataField", ::fbthrift::TType::Struct, 3);
        ::fbthrift::Serialize::rs_thrift_write(&self.MyDataField, p);
        p.write_field_end();
        p.write_field_begin("myEnum", ::fbthrift::TType::I32, 4);
        ::fbthrift::Serialize::rs_thrift_write(&self.myEnum, p);
        p.write_field_end();
        p.write_field_stop();
        p.write_struct_end();
    }
}

impl<P> ::fbthrift::Deserialize<P> for self::MyStruct
where
    P: ::fbthrift::ProtocolReader,
{
    #[inline]
    fn rs_thrift_read(p: &mut P) -> ::anyhow::Result<Self> {
        static FIELDS: &[::fbthrift::Field] = &[
            ::fbthrift::Field::new("MyDataField", ::fbthrift::TType::Struct, 3),
            ::fbthrift::Field::new("MyIntField", ::fbthrift::TType::I64, 1),
            ::fbthrift::Field::new("MyStringField", ::fbthrift::TType::String, 2),
            ::fbthrift::Field::new("myEnum", ::fbthrift::TType::I32, 4),
        ];

        #[allow(unused_mut)]
        let mut output = MyStruct::default();
        let _ = ::anyhow::Context::context(p.read_struct_begin(|_| ()), "Expected a MyStruct")?;
        let (_, mut fty, mut fid) = p.read_field_begin(|_| (), FIELDS)?;
        #[allow(unused_labels)]
        let fallback  = 'fastpath: {
            if (fty, fid) == (::fbthrift::TType::I64, 1) {
                output.MyIntField = ::anyhow::Context::context(::fbthrift::Deserialize::rs_thrift_read(p), ::fbthrift::errors::DeserializingFieldError { field: "MyIntField", strct: "MyStruct"})?;
                p.read_field_end()?;
            } else {
                break 'fastpath true;
            }
            (_, fty, fid) = p.read_field_begin(|_| (), FIELDS)?;
            if (fty, fid) == (::fbthrift::TType::String, 2) {
                output.MyStringField = ::anyhow::Context::context(::fbthrift::Deserialize::rs_thrift_read(p), ::fbthrift::errors::DeserializingFieldError { field: "MyStringField", strct: "MyStruct"})?;
                p.read_field_end()?;
            } else {
                break 'fastpath true;
            }
            (_, fty, fid) = p.read_field_begin(|_| (), FIELDS)?;
            if (fty, fid) == (::fbthrift::TType::Struct, 3) {
                output.MyDataField = ::anyhow::Context::context(::fbthrift::Deserialize::rs_thrift_read(p), ::fbthrift::errors::DeserializingFieldError { field: "MyDataField", strct: "MyStruct"})?;
                p.read_field_end()?;
            } else {
                break 'fastpath true;
            }
            (_, fty, fid) = p.read_field_begin(|_| (), FIELDS)?;
            if (fty, fid) == (::fbthrift::TType::I32, 4) {
                output.myEnum = ::anyhow::Context::context(::fbthrift::Deserialize::rs_thrift_read(p), ::fbthrift::errors::DeserializingFieldError { field: "myEnum", strct: "MyStruct"})?;
                p.read_field_end()?;
            } else {
                break 'fastpath true;
            }
            (_, fty, fid) = p.read_field_begin(|_| (), FIELDS)?;

            fty != ::fbthrift::TType::Stop
        };

        if fallback {
            loop {
                match (fty, fid) {
                    (::fbthrift::TType::Stop, _) => break,
                    (::fbthrift::TType::I64, 1) => output.MyIntField = ::anyhow::Context::context(::fbthrift::Deserialize::rs_thrift_read(p), ::fbthrift::errors::DeserializingFieldError { field: "MyIntField", strct: "MyStruct"})?,
                    (::fbthrift::TType::String, 2) => output.MyStringField = ::anyhow::Context::context(::fbthrift::Deserialize::rs_thrift_read(p), ::fbthrift::errors::DeserializingFieldError { field: "MyStringField", strct: "MyStruct"})?,
                    (::fbthrift::TType::Struct, 3) => output.MyDataField = ::anyhow::Context::context(::fbthrift::Deserialize::rs_thrift_read(p), ::fbthrift::errors::DeserializingFieldError { field: "MyDataField", strct: "MyStruct"})?,
                    (::fbthrift::TType::I32, 4) => output.myEnum = ::anyhow::Context::context(::fbthrift::Deserialize::rs_thrift_read(p), ::fbthrift::errors::DeserializingFieldError { field: "myEnum", strct: "MyStruct"})?,
                    (fty, _) => p.skip(fty)?,
                }
                p.read_field_end()?;
                (_, fty, fid) = p.read_field_begin(|_| (), FIELDS)?;
            }
        }
        p.read_struct_end()?;
        ::std::result::Result::Ok(output)

    }
}


impl ::fbthrift::metadata::ThriftAnnotations for MyStruct {
    fn get_structured_annotation<T: Sized + 'static>() -> ::std::option::Option<T> {
        #[allow(unused_variables)]
        let type_id = ::std::any::TypeId::of::<T>();

        ::std::option::Option::None
    }

    fn get_field_structured_annotation<T: Sized + 'static>(field_id: ::std::primitive::i16) -> ::std::option::Option<T> {
        #[allow(unused_variables)]
        let type_id = ::std::any::TypeId::of::<T>();

        #[allow(clippy::match_single_binding)]
        match field_id {
            1 => {
            },
            2 => {
            },
            3 => {
            },
            4 => {
            },
            _ => {}
        }

        ::std::option::Option::None
    }
}


#[allow(clippy::derivable_impls)]
impl ::std::default::Default for self::MyDataItem {
    fn default() -> Self {
        Self {
            _dot_dot_Default_default: self::dot_dot::OtherFields(()),
        }
    }
}

impl ::std::fmt::Debug for self::MyDataItem {
    fn fmt(&self, formatter: &mut ::std::fmt::Formatter) -> ::std::fmt::Result {
        formatter
            .debug_struct("MyDataItem")
            .finish()
    }
}

unsafe impl ::std::marker::Send for self::MyDataItem {}
unsafe impl ::std::marker::Sync for self::MyDataItem {}
impl ::std::marker::Unpin for self::MyDataItem {}
impl ::std::panic::RefUnwindSafe for self::MyDataItem {}
impl ::std::panic::UnwindSafe for self::MyDataItem {}

impl ::fbthrift::GetTType for self::MyDataItem {
    const TTYPE: ::fbthrift::TType = ::fbthrift::TType::Struct;
}

impl ::fbthrift::GetTypeNameType for self::MyDataItem {
    fn type_name_type() -> fbthrift::TypeNameType {
        ::fbthrift::TypeNameType::StructType
    }
}

impl<P> ::fbthrift::Serialize<P> for self::MyDataItem
where
    P: ::fbthrift::ProtocolWriter,
{
    #[inline]
    fn rs_thrift_write(&self, p: &mut P) {
        p.write_struct_begin("MyDataItem");
        p.write_field_stop();
        p.write_struct_end();
    }
}

impl<P> ::fbthrift::Deserialize<P> for self::MyDataItem
where
    P: ::fbthrift::ProtocolReader,
{
    #[inline]
    fn rs_thrift_read(p: &mut P) -> ::anyhow::Result<Self> {
        static FIELDS: &[::fbthrift::Field] = &[
        ];

        #[allow(unused_mut)]
        let mut output = MyDataItem::default();
        let _ = ::anyhow::Context::context(p.read_struct_begin(|_| ()), "Expected a MyDataItem")?;
        let (_, mut fty, mut fid) = p.read_field_begin(|_| (), FIELDS)?;
        #[allow(unused_labels)]
        let fallback  = 'fastpath: {

            fty != ::fbthrift::TType::Stop
        };

        if fallback {
            loop {
                match (fty, fid) {
                    (::fbthrift::TType::Stop, _) => break,
                    (fty, _) => p.skip(fty)?,
                }
                p.read_field_end()?;
                (_, fty, fid) = p.read_field_begin(|_| (), FIELDS)?;
            }
        }
        p.read_struct_end()?;
        ::std::result::Result::Ok(output)

    }
}


impl ::fbthrift::metadata::ThriftAnnotations for MyDataItem {
    fn get_structured_annotation<T: Sized + 'static>() -> ::std::option::Option<T> {
        #[allow(unused_variables)]
        let type_id = ::std::any::TypeId::of::<T>();

        ::std::option::Option::None
    }

    fn get_field_structured_annotation<T: Sized + 'static>(field_id: ::std::primitive::i16) -> ::std::option::Option<T> {
        #[allow(unused_variables)]
        let type_id = ::std::any::TypeId::of::<T>();

        #[allow(clippy::match_single_binding)]
        match field_id {
            _ => {}
        }

        ::std::option::Option::None
    }
}



impl ::std::default::Default for MyUnion {
    fn default() -> Self {
        Self::UnknownField(-1)
    }
}

impl ::fbthrift::GetTType for MyUnion {
    const TTYPE: ::fbthrift::TType = ::fbthrift::TType::Struct;
}

impl ::fbthrift::GetTypeNameType for self::MyUnion {
    fn type_name_type() -> fbthrift::TypeNameType {
        ::fbthrift::TypeNameType::UnionType
    }
}

impl<P> ::fbthrift::Serialize<P> for MyUnion
where
    P: ::fbthrift::ProtocolWriter,
{
    #[inline]
    fn rs_thrift_write(&self, p: &mut P) {
        p.write_struct_begin("MyUnion");
        match self {
            Self::myEnum(inner) => {
                p.write_field_begin("myEnum", ::fbthrift::TType::I32, 1);
                ::fbthrift::Serialize::rs_thrift_write(inner, p);
                p.write_field_end();
            }
            Self::myStruct(inner) => {
                p.write_field_begin("myStruct", ::fbthrift::TType::Struct, 2);
                ::fbthrift::Serialize::rs_thrift_write(inner, p);
                p.write_field_end();
            }
            Self::myDataItem(inner) => {
                p.write_field_begin("myDataItem", ::fbthrift::TType::Struct, 3);
                ::fbthrift::Serialize::rs_thrift_write(inner, p);
                p.write_field_end();
            }
            Self::UnknownField(_) => {}
        }
        p.write_field_stop();
        p.write_struct_end();
    }
}

impl<P> ::fbthrift::Deserialize<P> for MyUnion
where
    P: ::fbthrift::ProtocolReader,
{
    #[inline]
    fn rs_thrift_read(p: &mut P) -> ::anyhow::Result<Self> {
        static FIELDS: &[::fbthrift::Field] = &[
            ::fbthrift::Field::new("myDataItem", ::fbthrift::TType::Struct, 3),
            ::fbthrift::Field::new("myEnum", ::fbthrift::TType::I32, 1),
            ::fbthrift::Field::new("myStruct", ::fbthrift::TType::Struct, 2),
        ];
        let _ = ::anyhow::Context::context(p.read_struct_begin(|_| ()), "Expected a MyUnion")?;
        let mut once = false;
        let mut alt = ::std::option::Option::None;
        loop {
            let (_, fty, fid) = p.read_field_begin(|_| (), FIELDS)?;
            match (fty, fid as ::std::primitive::i32, once) {
                (::fbthrift::TType::Stop, _, _) => break,
                (::fbthrift::TType::I32, 1, false) => {
                    once = true;
                    alt = ::std::option::Option::Some(Self::myEnum(::anyhow::Context::context(::fbthrift::Deserialize::rs_thrift_read(p), ::fbthrift::errors::DeserializingFieldError { field: "myEnum", strct: "MyUnion"})?));
                }
                (::fbthrift::TType::Struct, 2, false) => {
                    once = true;
                    alt = ::std::option::Option::Some(Self::myStruct(::anyhow::Context::context(::fbthrift::Deserialize::rs_thrift_read(p), ::fbthrift::errors::DeserializingFieldError { field: "myStruct", strct: "MyUnion"})?));
                }
                (::fbthrift::TType::Struct, 3, false) => {
                    once = true;
                    alt = ::std::option::Option::Some(Self::myDataItem(::anyhow::Context::context(::fbthrift::Deserialize::rs_thrift_read(p), ::fbthrift::errors::DeserializingFieldError { field: "myDataItem", strct: "MyUnion"})?));
                }
                (fty, _, false) => p.skip(fty)?,
                (badty, badid, true) => return ::std::result::Result::Err(::std::convert::From::from(::fbthrift::ProtocolError::UnwantedExtraUnionField(
                    "MyUnion".to_string(),
                    badty,
                    badid,
                ))),
            }
            p.read_field_end()?;
        }
        p.read_struct_end()?;
        ::std::result::Result::Ok(alt.unwrap_or_default())
    }
}

impl MyUnion {
    /// Return current union variant name as a tuple of (Rust name, original name).
    pub fn variant_name(&self) -> ::std::option::Option<(&'static ::std::primitive::str, &'static ::std::primitive::str)> {
        match self {
            Self::myEnum(_) => ::std::option::Option::Some(("myEnum", "myEnum")),
            Self::myStruct(_) => ::std::option::Option::Some(("myStruct", "myStruct")),
            Self::myDataItem(_) => ::std::option::Option::Some(("myDataItem", "myDataItem")),
            Self::UnknownField(_) => ::std::option::Option::None,
        }
    }

    /// Return all union variant names as a tuple of (Rust name, original name).
    pub fn variant_names() -> &'static [(&'static ::std::primitive::str, &'static ::std::primitive::str)] {
        &[
            ("myEnum", "myEnum"),
            ("myStruct", "myStruct"),
            ("myDataItem", "myDataItem"),
        ]
    }
}

impl ::fbthrift::metadata::ThriftAnnotations for MyUnion {
    fn get_structured_annotation<T: Sized + 'static>() -> ::std::option::Option<T> {
        #[allow(unused_variables)]
        let type_id = ::std::any::TypeId::of::<T>();

        ::std::option::Option::None
    }

    fn get_field_structured_annotation<T: Sized + 'static>(field_id: ::std::primitive::i16) -> ::std::option::Option<T> {
        #[allow(unused_variables)]
        let type_id = ::std::any::TypeId::of::<T>();

        #[allow(clippy::match_single_binding)]
        match field_id {
            1 => {
            },
            2 => {
            },
            3 => {
            },
            _ => {}
        }

        ::std::option::Option::None
    }
}

#[allow(clippy::derivable_impls)]
impl ::std::default::Default for self::MyException {
    fn default() -> Self {
        Self {
            _dot_dot_Default_default: self::dot_dot::OtherFields(()),
        }
    }
}

impl ::std::fmt::Debug for self::MyException {
    fn fmt(&self, formatter: &mut ::std::fmt::Formatter) -> ::std::fmt::Result {
        formatter
            .debug_struct("MyException")
            .finish()
    }
}

unsafe impl ::std::marker::Send for self::MyException {}
unsafe impl ::std::marker::Sync for self::MyException {}
impl ::std::marker::Unpin for self::MyException {}
impl ::std::panic::RefUnwindSafe for self::MyException {}
impl ::std::panic::UnwindSafe for self::MyException {}

impl ::fbthrift::GetTType for self::MyException {
    const TTYPE: ::fbthrift::TType = ::fbthrift::TType::Struct;
}

impl ::fbthrift::GetTypeNameType for self::MyException {
    fn type_name_type() -> fbthrift::TypeNameType {
        ::fbthrift::TypeNameType::StructType
    }
}

impl<P> ::fbthrift::Serialize<P> for self::MyException
where
    P: ::fbthrift::ProtocolWriter,
{
    #[inline]
    fn rs_thrift_write(&self, p: &mut P) {
        p.write_struct_begin("MyException");
        p.write_field_stop();
        p.write_struct_end();
    }
}

impl<P> ::fbthrift::Deserialize<P> for self::MyException
where
    P: ::fbthrift::ProtocolReader,
{
    #[inline]
    fn rs_thrift_read(p: &mut P) -> ::anyhow::Result<Self> {
        static FIELDS: &[::fbthrift::Field] = &[
        ];

        #[allow(unused_mut)]
        let mut output = MyException::default();
        let _ = ::anyhow::Context::context(p.read_struct_begin(|_| ()), "Expected a MyException")?;
        let (_, mut fty, mut fid) = p.read_field_begin(|_| (), FIELDS)?;
        #[allow(unused_labels)]
        let fallback  = 'fastpath: {

            fty != ::fbthrift::TType::Stop
        };

        if fallback {
            loop {
                match (fty, fid) {
                    (::fbthrift::TType::Stop, _) => break,
                    (fty, _) => p.skip(fty)?,
                }
                p.read_field_end()?;
                (_, fty, fid) = p.read_field_begin(|_| (), FIELDS)?;
            }
        }
        p.read_struct_end()?;
        ::std::result::Result::Ok(output)

    }
}


impl ::fbthrift::metadata::ThriftAnnotations for MyException {
    fn get_structured_annotation<T: Sized + 'static>() -> ::std::option::Option<T> {
        #[allow(unused_variables)]
        let type_id = ::std::any::TypeId::of::<T>();

        ::std::option::Option::None
    }

    fn get_field_structured_annotation<T: Sized + 'static>(field_id: ::std::primitive::i16) -> ::std::option::Option<T> {
        #[allow(unused_variables)]
        let type_id = ::std::any::TypeId::of::<T>();

        #[allow(clippy::match_single_binding)]
        match field_id {
            _ => {}
        }

        ::std::option::Option::None
    }
}



mod dot_dot {
    #[derive(Copy, Clone, PartialEq, Eq, PartialOrd, Ord, Hash)]
    pub struct OtherFields(pub(crate) ());

    #[allow(dead_code)] // if serde isn't being used
    pub(super) fn default_for_serde_deserialize() -> OtherFields {
        OtherFields(())
    }
}

pub(crate) mod r#impl {
    use ::ref_cast::RefCast;

    #[derive(RefCast)]
    #[repr(transparent)]
    pub(crate) struct LocalImpl<T>(T);

    #[allow(unused)]
    pub(crate) fn rs_thrift_write<T, P>(value: &T, p: &mut P)
    where
        LocalImpl<T>: ::fbthrift::Serialize<P>,
        P: ::fbthrift::ProtocolWriter,
    {
        ::fbthrift::Serialize::rs_thrift_write(LocalImpl::ref_cast(value), p);
    }

    #[allow(unused)]
    pub(crate) fn rs_thrift_read<T, P>(p: &mut P) -> ::anyhow::Result<T>
    where
        LocalImpl<T>: ::fbthrift::Deserialize<P>,
        P: ::fbthrift::ProtocolReader,
    {
        let value: LocalImpl<T> = ::fbthrift::Deserialize::rs_thrift_read(p)?;
        ::std::result::Result::Ok(value.0)
    }
}


#[doc(hidden)]
#[deprecated]
#[allow(hidden_glob_reexports)]
pub mod __constructors {
    mod MyEnum {
        pub use crate::MyEnum;
    }
    pub use self::MyEnum::*;
}
