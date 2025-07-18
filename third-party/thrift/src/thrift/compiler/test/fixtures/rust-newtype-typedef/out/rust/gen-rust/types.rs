// @generated by Thrift for thrift/compiler/test/fixtures/rust-newtype-typedef/src/module.thrift
// This file is probably not the place you want to edit!


#![recursion_limit = "100000000"]
#![allow(non_camel_case_types, non_snake_case, non_upper_case_globals, unused_crate_dependencies, clippy::redundant_closure, clippy::type_complexity)]

#[allow(unused_imports)]
pub(crate) use crate as types;

#[derive(Default, Clone, Debug, PartialEq)]
pub struct MapType(pub ::sorted_vector_map::SortedVectorMap);

#[derive(Default, Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct BinType(pub ::smallvec::SmallVec<[u8; 16]>);

pub type Double = ::fbthrift::builtin_types::OrderedFloat<f64>;

#[derive(Default, Clone, Debug, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct BytesType(pub ::fbthrift::builtin_types::Bytes);

pub type binary_8247 = ::smallvec::SmallVec<[u8; 32]>;

pub type binary_9564 = ::fbthrift::builtin_types::Bytes;

pub type double_8056 = ::fbthrift::builtin_types::OrderedFloat<f64>;

#[derive(Clone, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct MyStruct {
    pub the_map: crate::types::MapType,
    pub the_bin: crate::types::BinType,
    pub inline_bin: ::smallvec::SmallVec<[u8; 32]>,
    pub the_bytes: crate::types::BytesType,
    pub inline_bytes: ::fbthrift::builtin_types::Bytes,
    pub floaty: ::fbthrift::builtin_types::OrderedFloat<f64>,
    pub doublefloaty: ::fbthrift::builtin_types::OrderedFloat<f64>,
    // This field forces `..Default::default()` when instantiating this
    // struct, to make code future-proof against new fields added later to
    // the definition in Thrift. If you don't want this, add the annotation
    // `@rust.Exhaustive` to the Thrift struct to eliminate this field.
    #[doc(hidden)]
    pub _dot_dot_Default_default: self::dot_dot::OtherFields,
}


impl ::fbthrift::GetTType for MapType {
    const TTYPE: ::fbthrift::TType = <::std::collections::BTreeMap<::std::primitive::i32, ::std::primitive::i32> as ::fbthrift::GetTType>::TTYPE;
}
impl<P> ::fbthrift::Serialize<P> for MapType
where
    P: ::fbthrift::ProtocolWriter,
{
    #[inline]
    fn rs_thrift_write(&self, p: &mut P) {
        crate::r#impl::rs_thrift_write(&self.0, p)
    }
}

impl<P> ::fbthrift::Deserialize<P> for MapType
where
    P: ::fbthrift::ProtocolReader,
{
    #[inline]
    fn rs_thrift_read(p: &mut P) -> ::anyhow::Result<Self> {
        crate::r#impl::rs_thrift_read(p).map(MapType)
    }
}

impl ::fbthrift::GetTType for BinType {
    const TTYPE: ::fbthrift::TType = <::std::vec::Vec<::std::primitive::u8> as ::fbthrift::GetTType>::TTYPE;
}
impl<P> ::fbthrift::Serialize<P> for BinType
where
    P: ::fbthrift::ProtocolWriter,
{
    #[inline]
    fn rs_thrift_write(&self, p: &mut P) {
        crate::r#impl::rs_thrift_write(&self.0, p)
    }
}

impl<P> ::fbthrift::Deserialize<P> for BinType
where
    P: ::fbthrift::ProtocolReader,
{
    #[inline]
    fn rs_thrift_read(p: &mut P) -> ::anyhow::Result<Self> {
        crate::r#impl::rs_thrift_read(p).map(BinType)
    }
}

impl ::fbthrift::GetTType for BytesType {
    const TTYPE: ::fbthrift::TType = <::std::vec::Vec<::std::primitive::u8> as ::fbthrift::GetTType>::TTYPE;
}

impl<P> ::fbthrift::Serialize<P> for BytesType
where
    P: ::fbthrift::ProtocolWriter,
{
    #[inline]
    fn rs_thrift_write(&self, p: &mut P) {
        self.0.rs_thrift_write(p)
    }
}

impl<P> ::fbthrift::Deserialize<P> for BytesType
where
    P: ::fbthrift::ProtocolReader,
{
    #[inline]
    fn rs_thrift_read(p: &mut P) -> ::anyhow::Result<Self> {
        ::fbthrift::Deserialize::rs_thrift_read(p).map(BytesType)
    }
}


#[allow(clippy::derivable_impls)]
impl ::std::default::Default for self::MyStruct {
    fn default() -> Self {
        Self {
            the_map: ::std::default::Default::default(),
            the_bin: ::std::default::Default::default(),
            inline_bin: ::std::default::Default::default(),
            the_bytes: ::std::default::Default::default(),
            inline_bytes: ::std::default::Default::default(),
            floaty: ::std::default::Default::default(),
            doublefloaty: ::std::default::Default::default(),
            _dot_dot_Default_default: self::dot_dot::OtherFields(()),
        }
    }
}

impl ::std::fmt::Debug for self::MyStruct {
    fn fmt(&self, formatter: &mut ::std::fmt::Formatter) -> ::std::fmt::Result {
        formatter
            .debug_struct("MyStruct")
            .field("the_map", &self.the_map)
            .field("the_bin", &self.the_bin)
            .field("inline_bin", &self.inline_bin)
            .field("the_bytes", &self.the_bytes)
            .field("inline_bytes", &self.inline_bytes)
            .field("floaty", &self.floaty)
            .field("doublefloaty", &self.doublefloaty)
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
        p.write_field_begin("the_map", ::fbthrift::TType::Map, 1);
        ::fbthrift::Serialize::rs_thrift_write(&self.the_map, p);
        p.write_field_end();
        p.write_field_begin("the_bin", ::fbthrift::TType::String, 2);
        ::fbthrift::Serialize::rs_thrift_write(&self.the_bin, p);
        p.write_field_end();
        p.write_field_begin("inline_bin", ::fbthrift::TType::String, 3);
        crate::r#impl::rs_thrift_write(&self.inline_bin, p);
        p.write_field_end();
        p.write_field_begin("the_bytes", ::fbthrift::TType::String, 4);
        ::fbthrift::Serialize::rs_thrift_write(&self.the_bytes, p);
        p.write_field_end();
        p.write_field_begin("inline_bytes", ::fbthrift::TType::String, 5);
        ::fbthrift::Serialize::rs_thrift_write(&self.inline_bytes, p);
        p.write_field_end();
        p.write_field_begin("floaty", ::fbthrift::TType::Double, 6);
        ::fbthrift::Serialize::rs_thrift_write(&self.floaty, p);
        p.write_field_end();
        p.write_field_begin("doublefloaty", ::fbthrift::TType::Double, 7);
        ::fbthrift::Serialize::rs_thrift_write(&self.doublefloaty, p);
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
            ::fbthrift::Field::new("doublefloaty", ::fbthrift::TType::Double, 7),
            ::fbthrift::Field::new("floaty", ::fbthrift::TType::Double, 6),
            ::fbthrift::Field::new("inline_bin", ::fbthrift::TType::String, 3),
            ::fbthrift::Field::new("inline_bytes", ::fbthrift::TType::String, 5),
            ::fbthrift::Field::new("the_bin", ::fbthrift::TType::String, 2),
            ::fbthrift::Field::new("the_bytes", ::fbthrift::TType::String, 4),
            ::fbthrift::Field::new("the_map", ::fbthrift::TType::Map, 1),
        ];

        #[allow(unused_mut)]
        let mut output = MyStruct::default();
        let _ = ::anyhow::Context::context(p.read_struct_begin(|_| ()), "Expected a MyStruct")?;
        let (_, mut fty, mut fid) = p.read_field_begin(|_| (), FIELDS)?;
        #[allow(unused_labels)]
        let fallback  = 'fastpath: {
            if (fty, fid) == (::fbthrift::TType::Map, 1) {
                output.the_map = ::anyhow::Context::context(::fbthrift::Deserialize::rs_thrift_read(p), ::fbthrift::errors::DeserializingFieldError { field: "the_map", strct: "MyStruct"})?;
                p.read_field_end()?;
            } else {
                break 'fastpath true;
            }
            (_, fty, fid) = p.read_field_begin(|_| (), FIELDS)?;
            if (fty, fid) == (::fbthrift::TType::String, 2) {
                output.the_bin = ::anyhow::Context::context(::fbthrift::Deserialize::rs_thrift_read(p), ::fbthrift::errors::DeserializingFieldError { field: "the_bin", strct: "MyStruct"})?;
                p.read_field_end()?;
            } else {
                break 'fastpath true;
            }
            (_, fty, fid) = p.read_field_begin(|_| (), FIELDS)?;
            if (fty, fid) == (::fbthrift::TType::String, 3) {
                output.inline_bin = ::anyhow::Context::context(crate::r#impl::rs_thrift_read(p), ::fbthrift::errors::DeserializingFieldError { field: "inline_bin", strct: "MyStruct"})?;
                p.read_field_end()?;
            } else {
                break 'fastpath true;
            }
            (_, fty, fid) = p.read_field_begin(|_| (), FIELDS)?;
            if (fty, fid) == (::fbthrift::TType::String, 4) {
                output.the_bytes = ::anyhow::Context::context(::fbthrift::Deserialize::rs_thrift_read(p), ::fbthrift::errors::DeserializingFieldError { field: "the_bytes", strct: "MyStruct"})?;
                p.read_field_end()?;
            } else {
                break 'fastpath true;
            }
            (_, fty, fid) = p.read_field_begin(|_| (), FIELDS)?;
            if (fty, fid) == (::fbthrift::TType::String, 5) {
                output.inline_bytes = ::anyhow::Context::context(::fbthrift::Deserialize::rs_thrift_read(p), ::fbthrift::errors::DeserializingFieldError { field: "inline_bytes", strct: "MyStruct"})?;
                p.read_field_end()?;
            } else {
                break 'fastpath true;
            }
            (_, fty, fid) = p.read_field_begin(|_| (), FIELDS)?;
            if (fty, fid) == (::fbthrift::TType::Double, 6) {
                output.floaty = ::anyhow::Context::context(::fbthrift::Deserialize::rs_thrift_read(p), ::fbthrift::errors::DeserializingFieldError { field: "floaty", strct: "MyStruct"})?;
                p.read_field_end()?;
            } else {
                break 'fastpath true;
            }
            (_, fty, fid) = p.read_field_begin(|_| (), FIELDS)?;
            if (fty, fid) == (::fbthrift::TType::Double, 7) {
                output.doublefloaty = ::anyhow::Context::context(::fbthrift::Deserialize::rs_thrift_read(p), ::fbthrift::errors::DeserializingFieldError { field: "doublefloaty", strct: "MyStruct"})?;
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
                    (::fbthrift::TType::Map, 1) => output.the_map = ::anyhow::Context::context(::fbthrift::Deserialize::rs_thrift_read(p), ::fbthrift::errors::DeserializingFieldError { field: "the_map", strct: "MyStruct"})?,
                    (::fbthrift::TType::String, 2) => output.the_bin = ::anyhow::Context::context(::fbthrift::Deserialize::rs_thrift_read(p), ::fbthrift::errors::DeserializingFieldError { field: "the_bin", strct: "MyStruct"})?,
                    (::fbthrift::TType::String, 3) => output.inline_bin = ::anyhow::Context::context(crate::r#impl::rs_thrift_read(p), ::fbthrift::errors::DeserializingFieldError { field: "inline_bin", strct: "MyStruct"})?,
                    (::fbthrift::TType::String, 4) => output.the_bytes = ::anyhow::Context::context(::fbthrift::Deserialize::rs_thrift_read(p), ::fbthrift::errors::DeserializingFieldError { field: "the_bytes", strct: "MyStruct"})?,
                    (::fbthrift::TType::String, 5) => output.inline_bytes = ::anyhow::Context::context(::fbthrift::Deserialize::rs_thrift_read(p), ::fbthrift::errors::DeserializingFieldError { field: "inline_bytes", strct: "MyStruct"})?,
                    (::fbthrift::TType::Double, 6) => output.floaty = ::anyhow::Context::context(::fbthrift::Deserialize::rs_thrift_read(p), ::fbthrift::errors::DeserializingFieldError { field: "floaty", strct: "MyStruct"})?,
                    (::fbthrift::TType::Double, 7) => output.doublefloaty = ::anyhow::Context::context(::fbthrift::Deserialize::rs_thrift_read(p), ::fbthrift::errors::DeserializingFieldError { field: "doublefloaty", strct: "MyStruct"})?,
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

        if type_id == ::std::any::TypeId::of::<rust__types::Ord>() {
            let mut tmp = ::std::option::Option::Some(rust__types::Ord {
                ..::std::default::Default::default()
            });
            let r: &mut dyn ::std::any::Any = &mut tmp;
            let r: &mut ::std::option::Option<T> = r.downcast_mut().unwrap();
            return r.take();
        }

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
            5 => {
            },
            6 => {
            },
            7 => {
            },
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

    impl<P> ::fbthrift::Serialize<P> for LocalImpl<::smallvec::SmallVec<[u8; 16]>>
    where
        P: ::fbthrift::ProtocolWriter,
    {
        #[inline]
        fn rs_thrift_write(&self, p: &mut P) {
            self.0.as_slice().rs_thrift_write(p)
        }
    }

    impl<P> ::fbthrift::Deserialize<P> for LocalImpl<::smallvec::SmallVec<[u8; 16]>>
    where
        P: ::fbthrift::ProtocolReader,
    {
        #[inline]
        fn rs_thrift_read(p: &mut P) -> ::anyhow::Result<Self> {
            p.read_binary()
        }
    }

    impl ::fbthrift::binary_type::BinaryType for LocalImpl<::smallvec::SmallVec<[u8; 16]>> {
        fn with_safe_capacity(capacity: usize) -> Self {
            LocalImpl(<::smallvec::SmallVec<[u8; 16]>>::with_capacity(capacity))
        }
        fn extend_from_slice(&mut self, other: &[::std::primitive::u8]) {
            self.0.extend_from_slice(other)
        }
        fn from_vec(vec: ::std::vec::Vec<::std::primitive::u8>) -> Self {
            LocalImpl(::std::convert::Into::into(vec))
        }
    }

    impl<P> ::fbthrift::Serialize<P> for LocalImpl<::smallvec::SmallVec<[u8; 32]>>
    where
        P: ::fbthrift::ProtocolWriter,
    {
        #[inline]
        fn rs_thrift_write(&self, p: &mut P) {
            self.0.as_slice().rs_thrift_write(p)
        }
    }

    impl<P> ::fbthrift::Deserialize<P> for LocalImpl<::smallvec::SmallVec<[u8; 32]>>
    where
        P: ::fbthrift::ProtocolReader,
    {
        #[inline]
        fn rs_thrift_read(p: &mut P) -> ::anyhow::Result<Self> {
            p.read_binary()
        }
    }

    impl ::fbthrift::binary_type::BinaryType for LocalImpl<::smallvec::SmallVec<[u8; 32]>> {
        fn with_safe_capacity(capacity: usize) -> Self {
            LocalImpl(<::smallvec::SmallVec<[u8; 32]>>::with_capacity(capacity))
        }
        fn extend_from_slice(&mut self, other: &[::std::primitive::u8]) {
            self.0.extend_from_slice(other)
        }
        fn from_vec(vec: ::std::vec::Vec<::std::primitive::u8>) -> Self {
            LocalImpl(::std::convert::Into::into(vec))
        }
    }

    impl<P> ::fbthrift::Serialize<P> for LocalImpl<::sorted_vector_map::SortedVectorMap<::std::primitive::i32, ::std::primitive::i32>>
    where
        P: ::fbthrift::ProtocolWriter,
    {
        #[inline]
        fn rs_thrift_write(&self, p: &mut P) {
            p.write_map_begin(
                <::std::primitive::i32 as ::fbthrift::GetTType>::TTYPE,
                <::std::primitive::i32 as ::fbthrift::GetTType>::TTYPE,
                self.0.len(),
            );
            for (k, v) in &self.0 {
                p.write_map_key_begin();
                ::fbthrift::Serialize::rs_thrift_write(k, p);
                p.write_map_value_begin();
                ::fbthrift::Serialize::rs_thrift_write(v, p);
            }
            p.write_map_end();
        }
    }

    impl<P> ::fbthrift::Deserialize<P> for LocalImpl<::sorted_vector_map::SortedVectorMap<::std::primitive::i32, ::std::primitive::i32>>
    where
        P: ::fbthrift::ProtocolReader,
    {
        #[inline]
        fn rs_thrift_read(p: &mut P) -> ::anyhow::Result<Self> {
            if <::std::primitive::i32 as ::fbthrift::GetTType>::TTYPE == ::fbthrift::TType::Void
                && <::std::primitive::i32 as ::fbthrift::GetTType>::TTYPE == ::fbthrift::TType::Void
            {
                ::anyhow::bail!(::fbthrift::ProtocolError::VoidCollectionElement);
            }

            let (_key_ty, _val_ty, len) = p.read_map_begin(P::min_size::<::std::primitive::i32>() + P::min_size::<::std::primitive::i32>())?;
            let mut map = <crate::types::MapType>::with_capacity(len.unwrap_or(0));

            if let ::std::option::Option::Some(0) = len {
                return ::std::result::Result::Ok(LocalImpl(map));
            }

            let mut idx = 0;
            loop {
                let more = p.read_map_key_begin()?;
                if !more {
                    break;
                }
                let k: ::std::primitive::i32 = ::fbthrift::Deserialize::rs_thrift_read(p)?;
                p.read_map_value_begin()?;
                let v: ::std::primitive::i32 = ::fbthrift::Deserialize::rs_thrift_read(p)?;
                p.read_map_value_end()?;
                map.insert(k, v);

                idx += 1;
                if ::fbthrift::protocol::should_break(len, more, idx) {
                    break;
                }
            }
            p.read_map_end()?;
            ::std::result::Result::Ok(LocalImpl(map))
        }
    }
}


#[doc(hidden)]
#[deprecated]
#[allow(hidden_glob_reexports)]
pub mod __constructors {
    mod BinType {
        pub use crate::BinType;
    }
    pub use self::BinType::*;
    mod BytesType {
        pub use crate::BytesType;
    }
    pub use self::BytesType::*;
    mod MapType {
        pub use crate::MapType;
    }
    pub use self::MapType::*;
}
