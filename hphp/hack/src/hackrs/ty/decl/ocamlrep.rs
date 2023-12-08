// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ocamlrep::FromOcamlRep;
use ocamlrep::ToOcamlRep;

use super::folded::*;
use super::ty::*;
use crate::reason::Reason;

// See comment on definition of `Enforceable`
impl<P: pos::Pos> ToOcamlRep for Enforceable<P> {
    fn to_ocamlrep<'a, A: ocamlrep::Allocator>(&'a self, alloc: &'a A) -> ocamlrep::Value<'a> {
        let mut block = alloc.block_with_size(2);
        let pos = self.as_ref().map_or_else(
            || alloc.add(oxidized_by_ref::pos::Pos::none()),
            |p| alloc.add(p),
        );
        alloc.set_field(&mut block, 0, pos);
        alloc.set_field(&mut block, 1, alloc.add_copy(self.is_some()));
        block.build()
    }
}

impl<P: pos::Pos> FromOcamlRep for Enforceable<P> {
    fn from_ocamlrep(value: ocamlrep::Value<'_>) -> Result<Self, ocamlrep::FromError> {
        let block = ocamlrep::from::expect_tuple(value, 2)?;
        let pos: P = ocamlrep::from::field(block, 0)?;
        let is_enforceable: bool = ocamlrep::from::field(block, 1)?;
        if is_enforceable {
            Ok(Self(Some(pos)))
        } else {
            Ok(Self(None))
        }
    }
}

// We need to hand-roll a ToOcamlRep impl for FoldedClass instead of deriving it
// in order to synthesize the `need_init` and `abstract` fields, which we derive
// from other information in the class in hackrs (whereas OCaml stores it
// redundantly).
impl<R: Reason> ToOcamlRep for FoldedClass<R> {
    fn to_ocamlrep<'a, A: ocamlrep::Allocator>(&'a self, alloc: &'a A) -> ocamlrep::Value<'a> {
        // Destructure to help ensure we convert every field.
        let Self {
            name,
            pos,
            kind,
            is_final,
            is_const,
            is_internal,
            is_xhp,
            has_xhp_keyword,
            support_dynamic_type,
            module,
            is_module_level_trait,
            tparams,
            where_constraints,
            substs,
            ancestors,
            props,
            static_props,
            methods,
            static_methods,
            consts,
            type_consts,
            xhp_enum_values,
            xhp_marked_empty,
            constructor,
            deferred_init_members,
            req_ancestors,
            req_ancestors_extends,
            req_class_ancestors,
            extends,
            sealed_whitelist,
            xhp_attr_deps,
            enum_type,
            decl_errors,
            docs_url,
            allow_multiple_instantiations,
        } = self;
        let need_init = self.has_concrete_constructor();
        let abstract_ = self.is_abstract();
        let mut block = alloc.block_with_size(37);
        alloc.set_field(&mut block, 0, alloc.add_copy(need_init));
        alloc.set_field(&mut block, 1, alloc.add_copy(abstract_));
        alloc.set_field(&mut block, 2, alloc.add(is_final));
        alloc.set_field(&mut block, 3, alloc.add(is_const));
        alloc.set_field(&mut block, 4, alloc.add(is_internal));
        alloc.set_field(&mut block, 5, alloc.add(deferred_init_members));
        alloc.set_field(&mut block, 6, alloc.add(kind));
        alloc.set_field(&mut block, 7, alloc.add(is_xhp));
        alloc.set_field(&mut block, 8, alloc.add(has_xhp_keyword));
        alloc.set_field(&mut block, 9, alloc.add(module));
        alloc.set_field(&mut block, 10, alloc.add(is_module_level_trait));
        alloc.set_field(&mut block, 11, alloc.add(name));
        alloc.set_field(&mut block, 12, alloc.add(pos));
        alloc.set_field(&mut block, 13, alloc.add(tparams));
        alloc.set_field(&mut block, 14, alloc.add(where_constraints));
        alloc.set_field(&mut block, 15, alloc.add(substs));
        alloc.set_field(&mut block, 16, alloc.add(consts));
        alloc.set_field(&mut block, 17, alloc.add(type_consts));
        alloc.set_field(&mut block, 18, alloc.add(props));
        alloc.set_field(&mut block, 19, alloc.add(static_props));
        alloc.set_field(&mut block, 20, alloc.add(methods));
        alloc.set_field(&mut block, 21, alloc.add(static_methods));
        alloc.set_field(&mut block, 22, alloc.add(constructor));
        alloc.set_field(&mut block, 23, alloc.add(ancestors));
        alloc.set_field(&mut block, 24, alloc.add(support_dynamic_type));
        alloc.set_field(&mut block, 25, alloc.add(req_ancestors));
        alloc.set_field(&mut block, 26, alloc.add(req_ancestors_extends));
        alloc.set_field(&mut block, 27, alloc.add(req_class_ancestors));
        alloc.set_field(&mut block, 28, alloc.add(extends));
        alloc.set_field(&mut block, 29, alloc.add(sealed_whitelist));
        alloc.set_field(&mut block, 30, alloc.add(xhp_attr_deps));
        alloc.set_field(&mut block, 31, alloc.add(xhp_enum_values));
        alloc.set_field(&mut block, 32, alloc.add(xhp_marked_empty));
        alloc.set_field(&mut block, 33, alloc.add(enum_type));
        alloc.set_field(&mut block, 34, alloc.add(decl_errors));
        alloc.set_field(&mut block, 35, alloc.add(docs_url));
        alloc.set_field(&mut block, 36, alloc.add(allow_multiple_instantiations));
        block.build()
    }
}

// Hand-written here because we lack the `need_init` and `abstract` fields.
// See comment on impl of ToOcamlRep for FoldedClass.
impl<R: Reason> FromOcamlRep for FoldedClass<R> {
    fn from_ocamlrep(value: ocamlrep::Value<'_>) -> Result<Self, ocamlrep::FromError> {
        let block = ocamlrep::from::expect_tuple(value, 37)?;
        Ok(Self {
            is_final: ocamlrep::from::field(block, 2)?,
            is_const: ocamlrep::from::field(block, 3)?,
            is_internal: ocamlrep::from::field(block, 4)?,
            deferred_init_members: ocamlrep::from::field(block, 5)?,
            kind: ocamlrep::from::field(block, 6)?,
            is_xhp: ocamlrep::from::field(block, 7)?,
            has_xhp_keyword: ocamlrep::from::field(block, 8)?,
            module: ocamlrep::from::field(block, 9)?,
            is_module_level_trait: ocamlrep::from::field(block, 10)?,
            name: ocamlrep::from::field(block, 11)?,
            pos: ocamlrep::from::field(block, 12)?,
            tparams: ocamlrep::from::field(block, 13)?,
            where_constraints: ocamlrep::from::field(block, 14)?,
            substs: ocamlrep::from::field(block, 15)?,
            consts: ocamlrep::from::field(block, 16)?,
            type_consts: ocamlrep::from::field(block, 17)?,
            props: ocamlrep::from::field(block, 18)?,
            static_props: ocamlrep::from::field(block, 19)?,
            methods: ocamlrep::from::field(block, 20)?,
            static_methods: ocamlrep::from::field(block, 21)?,
            constructor: ocamlrep::from::field(block, 22)?,
            ancestors: ocamlrep::from::field(block, 23)?,
            support_dynamic_type: ocamlrep::from::field(block, 24)?,
            req_ancestors: ocamlrep::from::field(block, 25)?,
            req_ancestors_extends: ocamlrep::from::field(block, 26)?,
            req_class_ancestors: ocamlrep::from::field(block, 27)?,
            extends: ocamlrep::from::field(block, 28)?,
            sealed_whitelist: ocamlrep::from::field(block, 29)?,
            xhp_attr_deps: ocamlrep::from::field(block, 30)?,
            xhp_enum_values: ocamlrep::from::field(block, 31)?,
            xhp_marked_empty: ocamlrep::from::field(block, 32)?,
            enum_type: ocamlrep::from::field(block, 33)?,
            decl_errors: ocamlrep::from::field(block, 34)?,
            docs_url: ocamlrep::from::field(block, 35)?,
            allow_multiple_instantiations: ocamlrep::from::field(block, 36)?,
        })
    }
}

/// It's not possible for us to derive ToOcamlRep for TshapeFieldName because we
/// represent it differently: OCaml includes positions in TshapeFieldName, but
/// we cannot (see the documentation on `TshapeFieldName` for rationale).
///
/// Instead, we store the positions in shape-map values, and feed them into this
/// function to produce the OCaml representation of `TshapeFieldName`.
fn shape_field_name_to_ocamlrep<'a, A: ocamlrep::Allocator, P: ToOcamlRep>(
    alloc: &'a A,
    name: &'a TshapeFieldName,
    field_name_pos: &'a ShapeFieldNamePos<P>,
) -> ocamlrep::Value<'a> {
    let simple_pos = || match field_name_pos {
        ShapeFieldNamePos::Simple(p) => p.to_ocamlrep(alloc),
        ShapeFieldNamePos::ClassConst(..) => panic!("expected ShapeFieldNamePos::Simple"),
    };
    match name {
        TshapeFieldName::TSFlitInt(x) => {
            let mut pos_string = alloc.block_with_size(2);
            alloc.set_field(&mut pos_string, 0, simple_pos());
            alloc.set_field(&mut pos_string, 1, alloc.add(x));
            let pos_string = pos_string.build();

            let mut block = alloc.block_with_size_and_tag(1usize, 0u8);
            alloc.set_field(&mut block, 0, pos_string);
            block.build()
        }
        TshapeFieldName::TSFlitStr(x) => {
            let mut pos_string = alloc.block_with_size(2);
            alloc.set_field(&mut pos_string, 0, simple_pos());
            alloc.set_field(&mut pos_string, 1, alloc.add(x));
            let pos_string = pos_string.build();

            let mut block = alloc.block_with_size_and_tag(1usize, 1u8);
            alloc.set_field(&mut block, 0, pos_string);
            block.build()
        }
        TshapeFieldName::TSFclassConst(cls, name) => {
            let (pos1, pos2) = match field_name_pos {
                ShapeFieldNamePos::ClassConst(p1, p2) => {
                    (p1.to_ocamlrep(alloc), p2.to_ocamlrep(alloc))
                }
                ShapeFieldNamePos::Simple(..) => panic!("expected ShapeFieldNamePos::ClassConst"),
            };

            let mut cls_pos_id = alloc.block_with_size(2);
            alloc.set_field(&mut cls_pos_id, 0, pos1);
            alloc.set_field(&mut cls_pos_id, 1, alloc.add(cls));
            let cls_pos_id = cls_pos_id.build();

            let mut const_pos_string = alloc.block_with_size(2);
            alloc.set_field(&mut const_pos_string, 0, pos2);
            alloc.set_field(&mut const_pos_string, 1, alloc.add(name));
            let const_pos_string = const_pos_string.build();

            let mut block = alloc.block_with_size_and_tag(2usize, 2u8);
            alloc.set_field(&mut block, 0, cls_pos_id);
            alloc.set_field(&mut block, 1, const_pos_string);
            block.build()
        }
    }
}

// See comment on `shape_field_name_to_ocamlrep`.
#[derive(FromOcamlRep)]
enum OcamlShapeFieldName<P> {
    Int(pos::Positioned<pos::Symbol, P>),
    Str(pos::Positioned<pos::Bytes, P>),
    ClassConst(
        pos::Positioned<pos::TypeName, P>,
        pos::Positioned<pos::Symbol, P>,
    ),
}

impl<R: Reason> ToOcamlRep for ShapeFieldType<R> {
    fn to_ocamlrep<'a, A: ocamlrep::Allocator>(&'a self, alloc: &'a A) -> ocamlrep::Value<'a> {
        let mut block = alloc.block_with_size_and_tag(2usize, 0u8);
        let ShapeFieldType {
            optional,
            ty,
            field_name_pos: _,
        } = self;
        alloc.set_field(&mut block, 0, alloc.add(optional));
        alloc.set_field(&mut block, 1, alloc.add(ty));
        block.build()
    }
}

impl<R: Reason> ToOcamlRep for ShapeType<R> {
    fn to_ocamlrep<'a, A: ocamlrep::Allocator>(&'a self, alloc: &'a A) -> ocamlrep::Value<'a> {
        let Self(shape_kind, shape_field_type_map) = &self;
        let map = if shape_field_type_map.is_empty() {
            ocamlrep::Value::int(0)
        } else {
            let len = shape_field_type_map.len();
            let mut iter = shape_field_type_map.iter().map(|(k, v)| {
                let k = shape_field_name_to_ocamlrep(alloc, k, &v.field_name_pos);
                (k, v.to_ocamlrep(alloc))
            });
            let (map, _) = ocamlrep::sorted_iter_to_ocaml_map(&mut iter, alloc, len);
            map
        };

        let mut block = alloc.block_with_size(3);
        // Note: we always set decl shapes to Missing_origin (0) as it is only for type aliases
        alloc.set_field(&mut block, 0, ocamlrep::Value::int(0));
        alloc.set_field(&mut block, 1, alloc.add(shape_kind));
        alloc.set_field(&mut block, 2, map);
        block.build()
    }
}

impl<R: Reason> FromOcamlRep for ShapeType<R> {
    fn from_ocamlrep(value: ocamlrep::Value<'_>) -> Result<Self, ocamlrep::FromError> {
        let block = ocamlrep::from::expect_tuple(value, 3)?;
        Ok(ShapeType(
            ocamlrep::from::field(block, 1)?,
            ocamlrep::vec_from_ocaml_map(block[2])?
                .into_iter()
                .map(|(k, (optional, ty))| match k {
                    OcamlShapeFieldName::Int(pos_id) => (
                        TshapeFieldName::TSFlitInt(pos_id.id()),
                        ShapeFieldType {
                            optional,
                            ty,
                            field_name_pos: ShapeFieldNamePos::Simple(pos_id.into_pos()),
                        },
                    ),
                    OcamlShapeFieldName::Str(pos_id) => (
                        TshapeFieldName::TSFlitStr(pos_id.id()),
                        ShapeFieldType {
                            optional,
                            ty,
                            field_name_pos: ShapeFieldNamePos::Simple(pos_id.into_pos()),
                        },
                    ),
                    OcamlShapeFieldName::ClassConst(cls_id, const_id) => (
                        TshapeFieldName::TSFclassConst(cls_id.id(), const_id.id()),
                        ShapeFieldType {
                            optional,
                            ty,
                            field_name_pos: ShapeFieldNamePos::ClassConst(
                                cls_id.into_pos(),
                                const_id.into_pos(),
                            ),
                        },
                    ),
                })
                .collect(),
        ))
    }
}

// Hand-written because we represent shape field names differently (see comment
// on `shape_field_name_to_ocamlrep`) and don't represent TanySentinel.
impl<R: Reason> ToOcamlRep for Ty_<R> {
    fn to_ocamlrep<'a, A: ocamlrep::Allocator>(&'a self, alloc: &'a A) -> ocamlrep::Value<'a> {
        match self {
            Ty_::Tthis => ocamlrep::Value::int(0),
            Ty_::Tapply(x) => {
                let mut block = alloc.block_with_size_and_tag(2usize, 0u8);
                alloc.set_field(&mut block, 0, alloc.add(&x.0));
                alloc.set_field(&mut block, 1, alloc.add(&x.1));
                block.build()
            }
            Ty_::Trefinement(x) => {
                let mut block = alloc.block_with_size_and_tag(2usize, 1u8);
                alloc.set_field(&mut block, 0, alloc.add(&x.ty));
                alloc.set_field(&mut block, 1, alloc.add(&x.refinement));
                block.build()
            }
            Ty_::Tmixed => ocamlrep::Value::int(1),
            Ty_::Twildcard => ocamlrep::Value::int(2),
            Ty_::Tlike(x) => {
                let mut block = alloc.block_with_size_and_tag(1usize, 2u8);
                alloc.set_field(&mut block, 0, alloc.add(x));
                block.build()
            }
            Ty_::Tany => {
                let mut block = alloc.block_with_size_and_tag(1usize, 3u8);
                alloc.set_field(&mut block, 0, alloc.add(&())); // TanySentinel
                block.build()
            }
            Ty_::Tnonnull => ocamlrep::Value::int(3),
            Ty_::Tdynamic => ocamlrep::Value::int(4),
            Ty_::Toption(x) => {
                let mut block = alloc.block_with_size_and_tag(1usize, 4u8);
                alloc.set_field(&mut block, 0, alloc.add(x));
                block.build()
            }
            Ty_::Tprim(x) => {
                let mut block = alloc.block_with_size_and_tag(1usize, 5u8);
                alloc.set_field(&mut block, 0, alloc.add(x));
                block.build()
            }
            Ty_::Tfun(x) => {
                let mut block = alloc.block_with_size_and_tag(1usize, 6u8);
                alloc.set_field(&mut block, 0, alloc.add(&**x));
                block.build()
            }
            Ty_::Ttuple(x) => {
                let mut block = alloc.block_with_size_and_tag(1usize, 7u8);
                alloc.set_field(&mut block, 0, alloc.add(&**x));
                block.build()
            }
            Ty_::Tshape(shape) => {
                let mut block = alloc.block_with_size_and_tag(1usize, 8u8);
                alloc.set_field(&mut block, 0, alloc.add(&**shape));
                block.build()
            }
            Ty_::Tgeneric(x) => {
                let mut block = alloc.block_with_size_and_tag(2usize, 9u8);
                alloc.set_field(&mut block, 0, alloc.add(&x.0));
                alloc.set_field(&mut block, 1, alloc.add(&x.1));
                block.build()
            }
            Ty_::Tunion(x) => {
                let mut block = alloc.block_with_size_and_tag(1usize, 10u8);
                alloc.set_field(&mut block, 0, alloc.add(&**x));
                block.build()
            }
            Ty_::Tintersection(x) => {
                let mut block = alloc.block_with_size_and_tag(1usize, 11u8);
                alloc.set_field(&mut block, 0, alloc.add(&**x));
                block.build()
            }
            Ty_::TvecOrDict(x) => {
                let mut block = alloc.block_with_size_and_tag(2usize, 12u8);
                alloc.set_field(&mut block, 0, alloc.add(&x.0));
                alloc.set_field(&mut block, 1, alloc.add(&x.1));
                block.build()
            }
            Ty_::Taccess(x) => {
                let mut block = alloc.block_with_size_and_tag(1usize, 13u8);
                alloc.set_field(&mut block, 0, alloc.add(&**x));
                block.build()
            }
        }
    }
}

// Hand-written because we represent shape field names differently (see comment
// on `shape_field_name_to_ocamlrep`) and don't represent TanySentinel.
impl<R: Reason> FromOcamlRep for Ty_<R> {
    fn from_ocamlrep(value: ocamlrep::Value<'_>) -> Result<Self, ocamlrep::FromError> {
        if value.is_int() {
            match value.as_int().unwrap() {
                0 => Ok(Ty_::Tthis),
                1 => Ok(Ty_::Tmixed),
                2 => Ok(Ty_::Twildcard),
                3 => Ok(Ty_::Tnonnull),
                4 => Ok(Ty_::Tdynamic),
                t => Err(ocamlrep::FromError::NullaryVariantTagOutOfRange { max: 4, actual: t }),
            }
        } else {
            let block = ocamlrep::from::expect_block(value)?;
            match block.tag() {
                0 => {
                    ocamlrep::from::expect_block_size(block, 2)?;
                    Ok(Ty_::Tapply(Box::new((
                        ocamlrep::from::field(block, 0)?,
                        ocamlrep::from::field(block, 1)?,
                    ))))
                }
                1 => {
                    ocamlrep::from::expect_block_size(block, 2)?;
                    Ok(Ty_::Trefinement(Box::new(TrefinementType {
                        ty: ocamlrep::from::field(block, 0)?,
                        refinement: ocamlrep::from::field(block, 1)?,
                    })))
                }
                2 => {
                    ocamlrep::from::expect_block_size(block, 1)?;
                    Ok(Ty_::Tlike(ocamlrep::from::field(block, 0)?))
                }
                3 => {
                    ocamlrep::from::expect_block_size(block, 1)?;
                    let () = ocamlrep::from::field(block, 0)?; // TanySentinel
                    Ok(Ty_::Tany)
                }
                4 => {
                    ocamlrep::from::expect_block_size(block, 1)?;
                    Ok(Ty_::Toption(ocamlrep::from::field(block, 0)?))
                }
                5 => {
                    ocamlrep::from::expect_block_size(block, 1)?;
                    Ok(Ty_::Tprim(ocamlrep::from::field(block, 0)?))
                }
                6 => {
                    ocamlrep::from::expect_block_size(block, 1)?;
                    Ok(Ty_::Tfun(ocamlrep::from::field(block, 0)?))
                }
                7 => {
                    ocamlrep::from::expect_block_size(block, 1)?;
                    Ok(Ty_::Ttuple(ocamlrep::from::field(block, 0)?))
                }
                8 => {
                    ocamlrep::from::expect_block_size(block, 1)?;
                    Ok(Ty_::Tshape(ocamlrep::from::field(block, 0)?))
                }
                9 => {
                    ocamlrep::from::expect_block_size(block, 2)?;
                    Ok(Ty_::Tgeneric(Box::new((
                        ocamlrep::from::field(block, 0)?,
                        ocamlrep::from::field(block, 1)?,
                    ))))
                }
                10 => {
                    ocamlrep::from::expect_block_size(block, 1)?;
                    Ok(Ty_::Tunion(ocamlrep::from::field(block, 0)?))
                }
                11 => {
                    ocamlrep::from::expect_block_size(block, 1)?;
                    Ok(Ty_::Tintersection(ocamlrep::from::field(block, 0)?))
                }
                12 => {
                    ocamlrep::from::expect_block_size(block, 2)?;
                    Ok(Ty_::TvecOrDict(Box::new((
                        ocamlrep::from::field(block, 0)?,
                        ocamlrep::from::field(block, 1)?,
                    ))))
                }
                13 => {
                    ocamlrep::from::expect_block_size(block, 1)?;
                    Ok(Ty_::Taccess(ocamlrep::from::field(block, 0)?))
                }
                t => Err(ocamlrep::FromError::BlockTagOutOfRange { max: 14, actual: t }),
            }
        }
    }
}
