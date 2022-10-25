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
    fn to_ocamlrep<'a, A: ocamlrep::Allocator>(
        &'a self,
        alloc: &'a A,
    ) -> ocamlrep::OpaqueValue<'a> {
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
    fn to_ocamlrep<'a, A: ocamlrep::Allocator>(
        &'a self,
        alloc: &'a A,
    ) -> ocamlrep::OpaqueValue<'a> {
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
        } = self;
        let need_init = self.has_concrete_constructor();
        let abstract_ = self.is_abstract();
        let mut block = alloc.block_with_size(34);
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
        alloc.set_field(&mut block, 10, alloc.add(name));
        alloc.set_field(&mut block, 11, alloc.add(pos));
        alloc.set_field(&mut block, 12, alloc.add(tparams));
        alloc.set_field(&mut block, 13, alloc.add(where_constraints));
        alloc.set_field(&mut block, 14, alloc.add(substs));
        alloc.set_field(&mut block, 15, alloc.add(consts));
        alloc.set_field(&mut block, 16, alloc.add(type_consts));
        alloc.set_field(&mut block, 17, alloc.add(props));
        alloc.set_field(&mut block, 18, alloc.add(static_props));
        alloc.set_field(&mut block, 19, alloc.add(methods));
        alloc.set_field(&mut block, 20, alloc.add(static_methods));
        alloc.set_field(&mut block, 21, alloc.add(constructor));
        alloc.set_field(&mut block, 22, alloc.add(ancestors));
        alloc.set_field(&mut block, 23, alloc.add(support_dynamic_type));
        alloc.set_field(&mut block, 24, alloc.add(req_ancestors));
        alloc.set_field(&mut block, 25, alloc.add(req_ancestors_extends));
        alloc.set_field(&mut block, 26, alloc.add(req_class_ancestors));
        alloc.set_field(&mut block, 27, alloc.add(extends));
        alloc.set_field(&mut block, 28, alloc.add(sealed_whitelist));
        alloc.set_field(&mut block, 29, alloc.add(xhp_attr_deps));
        alloc.set_field(&mut block, 30, alloc.add(xhp_enum_values));
        alloc.set_field(&mut block, 31, alloc.add(enum_type));
        alloc.set_field(&mut block, 32, alloc.add(decl_errors));
        alloc.set_field(&mut block, 33, alloc.add(docs_url));
        block.build()
    }
}

// Hand-written here because we lack the `need_init` and `abstract` fields.
// See comment on impl of ToOcamlRep for FoldedClass.
impl<R: Reason> FromOcamlRep for FoldedClass<R> {
    fn from_ocamlrep(value: ocamlrep::Value<'_>) -> Result<Self, ocamlrep::FromError> {
        let block = ocamlrep::from::expect_tuple(value, 34)?;
        Ok(Self {
            is_final: ocamlrep::from::field(block, 2)?,
            is_const: ocamlrep::from::field(block, 3)?,
            is_internal: ocamlrep::from::field(block, 4)?,
            deferred_init_members: ocamlrep::from::field(block, 5)?,
            kind: ocamlrep::from::field(block, 6)?,
            is_xhp: ocamlrep::from::field(block, 7)?,
            has_xhp_keyword: ocamlrep::from::field(block, 8)?,
            module: ocamlrep::from::field(block, 9)?,
            name: ocamlrep::from::field(block, 10)?,
            pos: ocamlrep::from::field(block, 11)?,
            tparams: ocamlrep::from::field(block, 12)?,
            where_constraints: ocamlrep::from::field(block, 13)?,
            substs: ocamlrep::from::field(block, 14)?,
            consts: ocamlrep::from::field(block, 15)?,
            type_consts: ocamlrep::from::field(block, 16)?,
            props: ocamlrep::from::field(block, 17)?,
            static_props: ocamlrep::from::field(block, 18)?,
            methods: ocamlrep::from::field(block, 19)?,
            static_methods: ocamlrep::from::field(block, 20)?,
            constructor: ocamlrep::from::field(block, 21)?,
            ancestors: ocamlrep::from::field(block, 22)?,
            support_dynamic_type: ocamlrep::from::field(block, 23)?,
            req_ancestors: ocamlrep::from::field(block, 24)?,
            req_ancestors_extends: ocamlrep::from::field(block, 25)?,
            req_class_ancestors: ocamlrep::from::field(block, 26)?,
            extends: ocamlrep::from::field(block, 27)?,
            sealed_whitelist: ocamlrep::from::field(block, 28)?,
            xhp_attr_deps: ocamlrep::from::field(block, 29)?,
            xhp_enum_values: ocamlrep::from::field(block, 30)?,
            enum_type: ocamlrep::from::field(block, 31)?,
            decl_errors: ocamlrep::from::field(block, 32)?,
            docs_url: ocamlrep::from::field(block, 33)?,
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
) -> ocamlrep::OpaqueValue<'a> {
    let simple_pos = || match field_name_pos {
        ShapeFieldNamePos::Simple(p) => p.to_ocamlrep(alloc),
        ShapeFieldNamePos::ClassConst(..) => panic!("expected ShapeFieldNamePos::Simple"),
    };
    match name {
        TshapeFieldName::TSFlitInt(x) => {
            let mut pos_string = alloc.block_with_size(2);
            alloc.set_field(&mut pos_string, 0, simple_pos());
            alloc.set_field(&mut pos_string, 1, alloc.add(&*x));
            let pos_string = pos_string.build();

            let mut block = alloc.block_with_size_and_tag(1usize, 0u8);
            alloc.set_field(&mut block, 0, pos_string);
            block.build()
        }
        TshapeFieldName::TSFlitStr(x) => {
            let mut pos_string = alloc.block_with_size(2);
            alloc.set_field(&mut pos_string, 0, simple_pos());
            alloc.set_field(&mut pos_string, 1, alloc.add(&*x));
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
            alloc.set_field(&mut cls_pos_id, 1, alloc.add(&*cls));
            let cls_pos_id = cls_pos_id.build();

            let mut const_pos_string = alloc.block_with_size(2);
            alloc.set_field(&mut const_pos_string, 0, pos2);
            alloc.set_field(&mut const_pos_string, 1, alloc.add(&*name));
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
    fn to_ocamlrep<'a, A: ocamlrep::Allocator>(
        &'a self,
        alloc: &'a A,
    ) -> ocamlrep::OpaqueValue<'a> {
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

// Hand-written because we represent shape field names differently (see comment
// on `shape_field_name_to_ocamlrep`) and don't represent TanySentinel.
impl<R: Reason> ToOcamlRep for Ty_<R> {
    fn to_ocamlrep<'a, A: ocamlrep::Allocator>(
        &'a self,
        alloc: &'a A,
    ) -> ocamlrep::OpaqueValue<'a> {
        match self {
            Ty_::Tthis => ocamlrep::OpaqueValue::int(0),
            Ty_::Tapply(x) => {
                let mut block = alloc.block_with_size_and_tag(2usize, 0u8);
                alloc.set_field(&mut block, 0, alloc.add(&x.0));
                alloc.set_field(&mut block, 1, alloc.add(&x.1));
                block.build()
            }
            Ty_::Trefinement(x) => {
                let mut block = alloc.block_with_size_and_tag(2usize, 1u8);
                alloc.set_field(&mut block, 0, alloc.add(&x.ty));
                alloc.set_field(&mut block, 1, alloc.add(&x.typeconsts));
                block.build()
            }
            Ty_::Tmixed => ocamlrep::OpaqueValue::int(1),
            Ty_::Tlike(x) => {
                let mut block = alloc.block_with_size_and_tag(1usize, 2u8);
                alloc.set_field(&mut block, 0, alloc.add(&*x));
                block.build()
            }
            Ty_::Tany => {
                let mut block = alloc.block_with_size_and_tag(1usize, 3u8);
                alloc.set_field(&mut block, 0, alloc.add(&())); // TanySentinel
                block.build()
            }
            Ty_::Terr => ocamlrep::OpaqueValue::int(2),
            Ty_::Tnonnull => ocamlrep::OpaqueValue::int(3),
            Ty_::Tdynamic => ocamlrep::OpaqueValue::int(4),
            Ty_::Toption(x) => {
                let mut block = alloc.block_with_size_and_tag(1usize, 4u8);
                alloc.set_field(&mut block, 0, alloc.add(&*x));
                block.build()
            }
            Ty_::Tprim(x) => {
                let mut block = alloc.block_with_size_and_tag(1usize, 5u8);
                alloc.set_field(&mut block, 0, alloc.add(&*x));
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
                let (shape_kind, shape_field_type_map): &(_, _) = shape;
                let map = if shape_field_type_map.is_empty() {
                    ocamlrep::OpaqueValue::int(0)
                } else {
                    let len = shape_field_type_map.len();
                    let mut iter = shape_field_type_map.iter().map(|(k, v)| {
                        let k = shape_field_name_to_ocamlrep(alloc, k, &v.field_name_pos);
                        (k, v.to_ocamlrep(alloc))
                    });
                    let (map, _) = ocamlrep::sorted_iter_to_ocaml_map(&mut iter, alloc, len);
                    map
                };

                let mut block = alloc.block_with_size_and_tag(2usize, 8u8);
                alloc.set_field(&mut block, 0, alloc.add(&*shape_kind));
                alloc.set_field(&mut block, 1, map);
                block.build()
            }
            Ty_::Tvar(ident) => {
                let mut block = alloc.block_with_size_and_tag(1usize, 9u8);
                alloc.set_field(&mut block, 0, alloc.add(&*ident));
                block.build()
            }
            Ty_::Tgeneric(x) => {
                let mut block = alloc.block_with_size_and_tag(2usize, 10u8);
                alloc.set_field(&mut block, 0, alloc.add(&x.0));
                alloc.set_field(&mut block, 1, alloc.add(&x.1));
                block.build()
            }
            Ty_::Tunion(x) => {
                let mut block = alloc.block_with_size_and_tag(1usize, 11u8);
                alloc.set_field(&mut block, 0, alloc.add(&**x));
                block.build()
            }
            Ty_::Tintersection(x) => {
                let mut block = alloc.block_with_size_and_tag(1usize, 12u8);
                alloc.set_field(&mut block, 0, alloc.add(&**x));
                block.build()
            }
            Ty_::TvecOrDict(x) => {
                let mut block = alloc.block_with_size_and_tag(2usize, 13u8);
                alloc.set_field(&mut block, 0, alloc.add(&x.0));
                alloc.set_field(&mut block, 1, alloc.add(&x.1));
                block.build()
            }
            Ty_::Taccess(x) => {
                let mut block = alloc.block_with_size_and_tag(1usize, 14u8);
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
        if value.is_immediate() {
            match value.as_int().unwrap() {
                0 => Ok(Ty_::Tthis),
                1 => Ok(Ty_::Tmixed),
                2 => Ok(Ty_::Terr),
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
                        typeconsts: ocamlrep::from::field(block, 1)?,
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
                    ocamlrep::from::expect_block_size(block, 2)?;
                    Ok(Ty_::Tshape(Box::new((
                        ocamlrep::from::field(block, 0)?,
                        ocamlrep::vec_from_ocaml_map(block[1])?
                            .into_iter()
                            .map(|(k, (optional, ty))| match k {
                                OcamlShapeFieldName::Int(pos_id) => (
                                    TshapeFieldName::TSFlitInt(pos_id.id()),
                                    ShapeFieldType {
                                        optional,
                                        ty,
                                        field_name_pos: ShapeFieldNamePos::Simple(
                                            pos_id.into_pos(),
                                        ),
                                    },
                                ),
                                OcamlShapeFieldName::Str(pos_id) => (
                                    TshapeFieldName::TSFlitStr(pos_id.id()),
                                    ShapeFieldType {
                                        optional,
                                        ty,
                                        field_name_pos: ShapeFieldNamePos::Simple(
                                            pos_id.into_pos(),
                                        ),
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
                    ))))
                }
                9 => {
                    ocamlrep::from::expect_block_size(block, 1)?;
                    Ok(Ty_::Tvar(ocamlrep::from::field(block, 0)?))
                }
                10 => {
                    ocamlrep::from::expect_block_size(block, 2)?;
                    Ok(Ty_::Tgeneric(Box::new((
                        ocamlrep::from::field(block, 0)?,
                        ocamlrep::from::field(block, 1)?,
                    ))))
                }
                11 => {
                    ocamlrep::from::expect_block_size(block, 1)?;
                    Ok(Ty_::Tunion(ocamlrep::from::field(block, 0)?))
                }
                12 => {
                    ocamlrep::from::expect_block_size(block, 1)?;
                    Ok(Ty_::Tintersection(ocamlrep::from::field(block, 0)?))
                }
                13 => {
                    ocamlrep::from::expect_block_size(block, 2)?;
                    Ok(Ty_::TvecOrDict(Box::new((
                        ocamlrep::from::field(block, 0)?,
                        ocamlrep::from::field(block, 1)?,
                    ))))
                }
                14 => {
                    ocamlrep::from::expect_block_size(block, 1)?;
                    Ok(Ty_::Taccess(ocamlrep::from::field(block, 0)?))
                }
                t => Err(ocamlrep::FromError::BlockTagOutOfRange { max: 14, actual: t }),
            }
        }
    }
}
