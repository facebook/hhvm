/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

use std::hash::Hash;
use std::hash::Hasher;
use std::sync::Arc;
use std::sync::Weak;

use type_id::TypeId;

use crate::error::InvalidTypeError;
use crate::nodes::EnumNode;
use crate::nodes::ListType;
use crate::nodes::MapType;
use crate::nodes::OpaqueAliasNode;
use crate::nodes::SetType;
use crate::nodes::StructNode;
use crate::nodes::UnionNode;
use crate::structured_node::StructuredNode;

/// Discriminant for [`TypeRef`] without the associated data.
#[derive(Copy, Clone, Debug, PartialEq, Eq, Hash)]
pub enum Kind {
    Bool,
    Byte,
    I16,
    I32,
    I64,
    Float,
    Double,
    String,
    Binary,
    Any,
    List,
    Set,
    Map,
    Struct,
    Union,
    Enum,
    OpaqueAlias,
}

impl Kind {
    pub fn name(self) -> &'static str {
        match self {
            Self::Bool => "bool",
            Self::Byte => "byte",
            Self::I16 => "i16",
            Self::I32 => "i32",
            Self::I64 => "i64",
            Self::Float => "float",
            Self::Double => "double",
            Self::String => "string",
            Self::Binary => "binary",
            Self::Any => "any",
            Self::List => "list",
            Self::Set => "set",
            Self::Map => "map",
            Self::Struct => "struct",
            Self::Union => "union",
            Self::Enum => "enum",
            Self::OpaqueAlias => "opaque_alias",
        }
    }
}

/// A reference to any Thrift type — primitive, container, or user-defined.
///
/// Cheaply cloneable. `Send + Sync`. User-defined variants hold a `Weak` handle
/// into the owning `TypeSystem` and must be navigated while that system is alive
/// (see module docs).
///
/// Equality semantics:
/// - Primitives: kind equality
/// - Containers: recursive structural equality
/// - User-defined: `Weak` pointer identity (one node per type within a `TypeSystem`)
#[derive(Clone, Debug)]
pub enum TypeRef {
    Bool,
    Byte,
    I16,
    I32,
    I64,
    Float,
    Double,
    String,
    Binary,
    Any,
    List(Arc<ListType>),
    Set(Arc<SetType>),
    Map(Arc<MapType>),
    Struct(Weak<StructNode>),
    Union(Weak<UnionNode>),
    Enum(Weak<EnumNode>),
    OpaqueAlias(Weak<OpaqueAliasNode>),
}

impl PartialEq for TypeRef {
    fn eq(&self, other: &Self) -> bool {
        match (self, other) {
            (Self::Bool, Self::Bool)
            | (Self::Byte, Self::Byte)
            | (Self::I16, Self::I16)
            | (Self::I32, Self::I32)
            | (Self::I64, Self::I64)
            | (Self::Float, Self::Float)
            | (Self::Double, Self::Double)
            | (Self::String, Self::String)
            | (Self::Binary, Self::Binary)
            | (Self::Any, Self::Any) => true,

            (Self::List(a), Self::List(b)) => Arc::ptr_eq(a, b) || a == b,
            (Self::Set(a), Self::Set(b)) => Arc::ptr_eq(a, b) || a == b,
            (Self::Map(a), Self::Map(b)) => Arc::ptr_eq(a, b) || a == b,

            (Self::Struct(a), Self::Struct(b)) => a.ptr_eq(b),
            (Self::Union(a), Self::Union(b)) => a.ptr_eq(b),
            (Self::Enum(a), Self::Enum(b)) => a.ptr_eq(b),
            (Self::OpaqueAlias(a), Self::OpaqueAlias(b)) => a.ptr_eq(b),

            _ => false,
        }
    }
}

impl Eq for TypeRef {}

impl Hash for TypeRef {
    fn hash<H: Hasher>(&self, state: &mut H) {
        std::mem::discriminant(self).hash(state);
        match self {
            Self::Bool
            | Self::Byte
            | Self::I16
            | Self::I32
            | Self::I64
            | Self::Float
            | Self::Double
            | Self::String
            | Self::Binary
            | Self::Any => {}

            Self::List(l) => l.hash(state),
            Self::Set(s) => s.hash(state),
            Self::Map(m) => m.hash(state),

            Self::Struct(s) => Weak::as_ptr(s).hash(state),
            Self::Union(u) => Weak::as_ptr(u).hash(state),
            Self::Enum(e) => Weak::as_ptr(e).hash(state),
            Self::OpaqueAlias(o) => Weak::as_ptr(o).hash(state),
        }
    }
}

impl TypeRef {
    pub fn list_of(element: TypeRef) -> TypeRef {
        TypeRef::List(Arc::new(ListType::new(element)))
    }

    pub fn set_of(element: TypeRef) -> TypeRef {
        TypeRef::Set(Arc::new(SetType::new(element)))
    }

    pub fn map_of(key: TypeRef, value: TypeRef) -> TypeRef {
        TypeRef::Map(Arc::new(MapType::new(key, value)))
    }

    pub fn kind(&self) -> Kind {
        match self {
            Self::Bool => Kind::Bool,
            Self::Byte => Kind::Byte,
            Self::I16 => Kind::I16,
            Self::I32 => Kind::I32,
            Self::I64 => Kind::I64,
            Self::Float => Kind::Float,
            Self::Double => Kind::Double,
            Self::String => Kind::String,
            Self::Binary => Kind::Binary,
            Self::Any => Kind::Any,
            Self::List(_) => Kind::List,
            Self::Set(_) => Kind::Set,
            Self::Map(_) => Kind::Map,
            Self::Struct(_) => Kind::Struct,
            Self::Union(_) => Kind::Union,
            Self::Enum(_) => Kind::Enum,
            Self::OpaqueAlias(_) => Kind::OpaqueAlias,
        }
    }

    pub fn is_bool(&self) -> bool {
        self.kind() == Kind::Bool
    }
    pub fn is_byte(&self) -> bool {
        self.kind() == Kind::Byte
    }
    pub fn is_i16(&self) -> bool {
        self.kind() == Kind::I16
    }
    pub fn is_i32(&self) -> bool {
        self.kind() == Kind::I32
    }
    pub fn is_i64(&self) -> bool {
        self.kind() == Kind::I64
    }
    pub fn is_float(&self) -> bool {
        self.kind() == Kind::Float
    }
    pub fn is_double(&self) -> bool {
        self.kind() == Kind::Double
    }
    pub fn is_string(&self) -> bool {
        self.kind() == Kind::String
    }
    pub fn is_binary(&self) -> bool {
        self.kind() == Kind::Binary
    }
    pub fn is_any(&self) -> bool {
        self.kind() == Kind::Any
    }
    pub fn is_list(&self) -> bool {
        self.kind() == Kind::List
    }
    pub fn is_set(&self) -> bool {
        self.kind() == Kind::Set
    }
    pub fn is_map(&self) -> bool {
        self.kind() == Kind::Map
    }
    pub fn is_struct(&self) -> bool {
        self.kind() == Kind::Struct
    }
    pub fn is_union(&self) -> bool {
        self.kind() == Kind::Union
    }
    pub fn is_enum(&self) -> bool {
        self.kind() == Kind::Enum
    }
    pub fn is_opaque_alias(&self) -> bool {
        self.kind() == Kind::OpaqueAlias
    }

    pub fn is_structured(&self) -> bool {
        self.is_struct() || self.is_union()
    }

    pub fn as_list(&self) -> Result<&ListType, InvalidTypeError> {
        match self {
            Self::List(l) => Ok(l),
            _ => Err(self.wrong_kind("list")),
        }
    }

    pub fn as_set(&self) -> Result<&SetType, InvalidTypeError> {
        match self {
            Self::Set(s) => Ok(s),
            _ => Err(self.wrong_kind("set")),
        }
    }

    pub fn as_map(&self) -> Result<&MapType, InvalidTypeError> {
        match self {
            Self::Map(m) => Ok(m),
            _ => Err(self.wrong_kind("map")),
        }
    }

    pub fn as_struct(&self) -> Result<Arc<StructNode>, InvalidTypeError> {
        match self {
            Self::Struct(s) => Ok(upgrade(s, "Struct")),
            _ => Err(self.wrong_kind("struct")),
        }
    }

    pub fn as_union(&self) -> Result<Arc<UnionNode>, InvalidTypeError> {
        match self {
            Self::Union(u) => Ok(upgrade(u, "Union")),
            _ => Err(self.wrong_kind("union")),
        }
    }

    pub fn as_enum(&self) -> Result<Arc<EnumNode>, InvalidTypeError> {
        match self {
            Self::Enum(e) => Ok(upgrade(e, "Enum")),
            _ => Err(self.wrong_kind("enum")),
        }
    }

    pub fn as_opaque_alias(&self) -> Result<Arc<OpaqueAliasNode>, InvalidTypeError> {
        match self {
            Self::OpaqueAlias(o) => Ok(upgrade(o, "OpaqueAlias")),
            _ => Err(self.wrong_kind("opaque_alias")),
        }
    }

    pub fn as_structured(&self) -> Result<Arc<dyn StructuredNode>, InvalidTypeError> {
        match self {
            Self::Struct(s) => {
                let node: Arc<StructNode> = upgrade(s, "Struct");
                Ok(node)
            }
            Self::Union(u) => {
                let node: Arc<UnionNode> = upgrade(u, "Union");
                Ok(node)
            }
            _ => Err(self.wrong_kind("struct or union")),
        }
    }

    /// Produces the serializable `TypeId` for this type.
    pub fn id(&self) -> TypeId {
        match self {
            Self::Bool => TypeId::boolType(Default::default()),
            Self::Byte => TypeId::byteType(Default::default()),
            Self::I16 => TypeId::i16Type(Default::default()),
            Self::I32 => TypeId::i32Type(Default::default()),
            Self::I64 => TypeId::i64Type(Default::default()),
            Self::Float => TypeId::floatType(Default::default()),
            Self::Double => TypeId::doubleType(Default::default()),
            Self::String => TypeId::stringType(Default::default()),
            Self::Binary => TypeId::binaryType(Default::default()),
            Self::Any => TypeId::anyType(Default::default()),
            Self::List(l) => TypeId::listType(type_id::ListTypeId {
                elementType: Some(Box::new(l.element_type().id())),
                ..Default::default()
            }),
            Self::Set(s) => TypeId::setType(type_id::SetTypeId {
                elementType: Some(Box::new(s.element_type().id())),
                ..Default::default()
            }),
            Self::Map(m) => TypeId::mapType(type_id::MapTypeId {
                keyType: Some(Box::new(m.key_type().id())),
                valueType: Some(Box::new(m.value_type().id())),
                ..Default::default()
            }),
            Self::Struct(s) => TypeId::userDefinedType(upgrade(s, "Struct").uri().to_owned()),
            Self::Union(u) => TypeId::userDefinedType(upgrade(u, "Union").uri().to_owned()),
            Self::Enum(e) => TypeId::userDefinedType(upgrade(e, "Enum").uri().to_owned()),
            Self::OpaqueAlias(o) => {
                TypeId::userDefinedType(upgrade(o, "OpaqueAlias").uri().to_owned())
            }
        }
    }

    fn wrong_kind(&self, expected: &'static str) -> InvalidTypeError {
        InvalidTypeError::WrongKind {
            expected,
            actual: self.kind().name(),
        }
    }
}

/// Upgrades a `Weak` edge to its owning node.
///
/// Panics if the owning `TypeSystem` has been dropped: a `TypeRef` must only be
/// navigated while its type system is alive (see module docs).
fn upgrade<T>(weak: &Weak<T>, kind: &'static str) -> Arc<T> {
    weak.upgrade()
        .unwrap_or_else(|| panic!("TypeRef::{kind} navigated after its TypeSystem was dropped"))
}

impl From<DefinitionRef> for TypeRef {
    fn from(def: DefinitionRef) -> Self {
        match def {
            DefinitionRef::Struct(s) => Self::Struct(Arc::downgrade(&s)),
            DefinitionRef::Union(u) => Self::Union(Arc::downgrade(&u)),
            DefinitionRef::Enum(e) => Self::Enum(Arc::downgrade(&e)),
            DefinitionRef::OpaqueAlias(o) => Self::OpaqueAlias(Arc::downgrade(&o)),
        }
    }
}

/// Discriminant for the 4 `DefinitionRef` variants.
#[derive(Copy, Clone, Debug, PartialEq, Eq, Hash)]
pub enum DefinitionKind {
    Struct,
    Union,
    Enum,
    OpaqueAlias,
}

/// A held reference to a user-defined type node (struct, union, enum, or opaque
/// alias).
///
/// Unlike the `Weak` edges inside [`TypeRef`], a `DefinitionRef` holds a strong
/// `Arc`, so it is guaranteed live and its accessors return borrows without
/// upgrading. This is the type returned by `TypeSystem::get`. Equality and
/// hashing use `Arc` pointer identity.
#[derive(Clone, Debug)]
pub enum DefinitionRef {
    Struct(Arc<StructNode>),
    Union(Arc<UnionNode>),
    Enum(Arc<EnumNode>),
    OpaqueAlias(Arc<OpaqueAliasNode>),
}

impl DefinitionRef {
    pub fn kind(&self) -> DefinitionKind {
        match self {
            Self::Struct(_) => DefinitionKind::Struct,
            Self::Union(_) => DefinitionKind::Union,
            Self::Enum(_) => DefinitionKind::Enum,
            Self::OpaqueAlias(_) => DefinitionKind::OpaqueAlias,
        }
    }

    pub fn is_struct(&self) -> bool {
        matches!(self, Self::Struct(_))
    }
    pub fn is_union(&self) -> bool {
        matches!(self, Self::Union(_))
    }
    pub fn is_enum(&self) -> bool {
        matches!(self, Self::Enum(_))
    }
    pub fn is_opaque_alias(&self) -> bool {
        matches!(self, Self::OpaqueAlias(_))
    }
    pub fn is_structured(&self) -> bool {
        self.is_struct() || self.is_union()
    }

    pub fn uri(&self) -> &str {
        match self {
            Self::Struct(s) => s.uri(),
            Self::Union(u) => u.uri(),
            Self::Enum(e) => e.uri(),
            Self::OpaqueAlias(o) => o.uri(),
        }
    }

    pub fn as_struct(&self) -> Result<&StructNode, InvalidTypeError> {
        match self {
            Self::Struct(s) => Ok(s),
            _ => Err(self.wrong_kind("struct")),
        }
    }

    pub fn as_union(&self) -> Result<&UnionNode, InvalidTypeError> {
        match self {
            Self::Union(u) => Ok(u),
            _ => Err(self.wrong_kind("union")),
        }
    }

    pub fn as_enum(&self) -> Result<&EnumNode, InvalidTypeError> {
        match self {
            Self::Enum(e) => Ok(e),
            _ => Err(self.wrong_kind("enum")),
        }
    }

    pub fn as_opaque_alias(&self) -> Result<&OpaqueAliasNode, InvalidTypeError> {
        match self {
            Self::OpaqueAlias(o) => Ok(o),
            _ => Err(self.wrong_kind("opaque_alias")),
        }
    }

    pub fn as_structured(&self) -> Result<&dyn StructuredNode, InvalidTypeError> {
        match self {
            Self::Struct(s) => Ok(s.as_ref()),
            Self::Union(u) => Ok(u.as_ref()),
            _ => Err(self.wrong_kind("struct or union")),
        }
    }

    fn wrong_kind(&self, expected: &'static str) -> InvalidTypeError {
        InvalidTypeError::WrongKind {
            expected,
            actual: match self {
                Self::Struct(_) => "struct",
                Self::Union(_) => "union",
                Self::Enum(_) => "enum",
                Self::OpaqueAlias(_) => "opaque_alias",
            },
        }
    }
}

impl PartialEq for DefinitionRef {
    fn eq(&self, other: &Self) -> bool {
        match (self, other) {
            (Self::Struct(a), Self::Struct(b)) => Arc::ptr_eq(a, b),
            (Self::Union(a), Self::Union(b)) => Arc::ptr_eq(a, b),
            (Self::Enum(a), Self::Enum(b)) => Arc::ptr_eq(a, b),
            (Self::OpaqueAlias(a), Self::OpaqueAlias(b)) => Arc::ptr_eq(a, b),
            _ => false,
        }
    }
}

impl Eq for DefinitionRef {}

impl Hash for DefinitionRef {
    fn hash<H: Hasher>(&self, state: &mut H) {
        std::mem::discriminant(self).hash(state);
        match self {
            Self::Struct(s) => Arc::as_ptr(s).hash(state),
            Self::Union(u) => Arc::as_ptr(u).hash(state),
            Self::Enum(e) => Arc::as_ptr(e).hash(state),
            Self::OpaqueAlias(o) => Arc::as_ptr(o).hash(state),
        }
    }
}

#[cfg(test)]
mod tests {
    use std::collections::HashMap;

    use super::*;

    fn make_struct(uri: &str) -> Arc<StructNode> {
        Arc::new(StructNode::new_shell(uri.to_owned(), false, HashMap::new()))
    }

    fn make_union(uri: &str) -> Arc<UnionNode> {
        Arc::new(UnionNode::new_shell(uri.to_owned(), false, HashMap::new()))
    }

    fn make_enum(uri: &str) -> Arc<crate::nodes::EnumNode> {
        Arc::new(crate::nodes::EnumNode::new(uri.to_owned(), vec![], HashMap::new()).unwrap())
    }

    fn make_opaque_alias(uri: &str) -> Arc<OpaqueAliasNode> {
        Arc::new(OpaqueAliasNode::new_shell(uri.to_owned(), HashMap::new()))
    }

    #[test]
    fn kind_returns_correct_variant() {
        assert_eq!(TypeRef::Bool.kind(), Kind::Bool);
        assert_eq!(TypeRef::I32.kind(), Kind::I32);
        assert_eq!(TypeRef::String.kind(), Kind::String);
        assert_eq!(TypeRef::Any.kind(), Kind::Any);
        assert_eq!(TypeRef::list_of(TypeRef::Bool).kind(), Kind::List);
        assert_eq!(TypeRef::set_of(TypeRef::Bool).kind(), Kind::Set);
        assert_eq!(
            TypeRef::map_of(TypeRef::I32, TypeRef::String).kind(),
            Kind::Map
        );
        let sn = make_struct("S");
        assert_eq!(TypeRef::Struct(Arc::downgrade(&sn)).kind(), Kind::Struct);
        let en = make_enum("E");
        assert_eq!(TypeRef::Enum(Arc::downgrade(&en)).kind(), Kind::Enum);
    }

    #[test]
    fn as_struct_success_and_wrong_kind() {
        let node = make_struct("test::S");
        let r = TypeRef::Struct(Arc::downgrade(&node));
        assert_eq!(r.as_struct().unwrap().uri(), "test::S");
        assert!(TypeRef::Bool.as_struct().is_err());
    }

    #[test]
    fn as_list_set_map() {
        let list = TypeRef::list_of(TypeRef::Bool);
        assert_eq!(list.as_list().unwrap().element_type().kind(), Kind::Bool);
        let set = TypeRef::set_of(TypeRef::I64);
        assert_eq!(set.as_set().unwrap().element_type().kind(), Kind::I64);
        let map = TypeRef::map_of(TypeRef::String, TypeRef::I32);
        assert_eq!(map.as_map().unwrap().key_type().kind(), Kind::String);
    }

    #[test]
    fn as_structured_accepts_struct_and_union() {
        let sn = make_struct("S");
        let s = TypeRef::Struct(Arc::downgrade(&sn));
        assert!(s.as_structured().is_ok());
        let un = make_union("U");
        let u = TypeRef::Union(Arc::downgrade(&un));
        assert!(u.as_structured().is_ok());
        let en = make_enum("E");
        assert!(TypeRef::Enum(Arc::downgrade(&en)).as_structured().is_err());
    }

    #[test]
    fn definition_ref_uri_and_kind() {
        let node = make_enum("test::E");
        let def = DefinitionRef::Enum(Arc::clone(&node));
        assert_eq!(def.uri(), "test::E");
        assert!(def.is_enum());
        assert!(!def.is_struct());
    }

    #[test]
    fn definition_ref_as_struct_wrong_kind() {
        let def = DefinitionRef::Union(make_union("U"));
        assert!(matches!(
            def.as_struct(),
            Err(InvalidTypeError::WrongKind {
                expected: "struct",
                ..
            })
        ));
    }

    #[test]
    fn is_predicates() {
        assert!(TypeRef::Bool.is_bool());
        assert!(TypeRef::I32.is_i32());
        assert!(TypeRef::list_of(TypeRef::Bool).is_list());
        let sn = make_struct("S");
        let s = TypeRef::Struct(Arc::downgrade(&sn));
        assert!(s.is_struct());
        assert!(s.is_structured());
        assert!(!TypeRef::Bool.is_structured());
    }
}
