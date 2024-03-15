use bstr::ByteSlice;
use ir::instr;
use ir::Attr;
use ir::Attribute;
use ir::Class;
use ir::ClassName;
use ir::Constant;
use ir::Constraint;
use ir::FuncBuilder;
use ir::Immediate;
use ir::Instr;
use ir::LocalId;
use ir::MemberOpBuilder;
use ir::Method;
use ir::MethodFlags;
use ir::MethodName;
use ir::Param;
use ir::PropName;
use ir::Property;
use ir::TypeConstant;
use ir::TypeConstraintFlags;
use ir::TypeInfo;
use ir::TypedValue;
use ir::Visibility;
use log::trace;

use crate::class::IsStatic;

/// This indicates a static property that started life as a class constant.
pub(crate) const INFER_CONSTANT: &str = "constant";
pub(crate) const INFER_TYPE_CONSTANT: &str = "type_constant";

pub(crate) const THIS_AS_PROPERTY: &str = "this";

fn typed_value_into_string(tv: &TypedValue) -> Option<String> {
    let ostr = tv
        .get_string()
        .map(|sid| sid.as_bytes().as_bstr().to_string());
    trace!("tv {:?} -> {:?}", tv, ostr);
    ostr
}

fn typed_value_into_strings(tv: &TypedValue) -> Option<Vec<String>> {
    let ovs = if let TypedValue::Vec(tvs) = tv {
        tvs.iter().map(typed_value_into_string).collect()
    } else {
        None
    };
    trace!("tvs {:?} -> {:?}", tv, ovs);
    ovs
}

fn compute_tc_attribute(typed_value: &TypedValue) -> Option<String> {
    match typed_value {
        TypedValue::Dict(dict) => {
            let kind_key = ir::TypedValue::String(ir::intern("kind").as_bytes());
            ir::dict_get(dict, &kind_key)
                .and_then(|tv| tv.get_int())
                .and_then(|i| {
                    // TODO(dpichardie) Incomplete support for type constant definitions.
                    // One should add T_int, T_bool, ...
                    let unresolved: i64 = ir::TypeStructureKind::T_unresolved.into();
                    let type_access: i64 = ir::TypeStructureKind::T_typeaccess.into();
                    if i == unresolved {
                        let class_name = ir::TypedValue::String(ir::intern("classname").as_bytes());
                        ir::dict_get(dict, &class_name).and_then(typed_value_into_string)
                    } else if i == type_access {
                        let root_name = ir::TypedValue::String(ir::intern("root_name").as_bytes());
                        let access_list =
                            ir::TypedValue::String(ir::intern("access_list").as_bytes());
                        let root_name =
                            ir::dict_get(dict, &root_name).and_then(typed_value_into_string);
                        let access_list =
                            ir::dict_get(dict, &access_list).and_then(typed_value_into_strings);
                        match (root_name, access_list) {
                            (Some(root), Some(access)) => {
                                if access.is_empty() {
                                    Some(root)
                                } else {
                                    Some(format!("{}::{}", root, access.join("::")))
                                }
                            }
                            (_, _) => None,
                        }
                    } else {
                        None
                    }
                })
        }
        _ => None,
    }
}

pub(crate) fn lower_class(mut class: Class) -> Class {
    if !class.ctx_constants.is_empty() {
        textual_todo! {
            trace!("TODO: class.ctx_constants");
        }
    }

    if !class.upper_bounds.is_empty() {
        textual_todo! {
            trace!("TODO: class.upper_bounds");
        }
    }

    if class.flags.is_closure_class() {
        // We want closure classes to always contain a 'this' but in HHVM it's
        // implicit so add a fake 'this' property. Unfortunately we have no good
        // way to identify the original class type. We could scrape it out of
        // the name but that's fragile. For closures of functions this will be
        // null.

        // (strip the leading '$')
        let name = PropName::intern(THIS_AS_PROPERTY);
        class.properties.insert(
            0,
            Property {
                name,
                flags: Attr::AttrNone,
                attributes: Vec::default(),
                visibility: Visibility::Private,
                initial_value: None,
                type_info: TypeInfo::default(),
                doc_comment: Default::default(),
            },
        );
    }

    for method in &mut class.methods {
        if method.name.is_86pinit() {
            // We want 86pinit to be 'instance' but hackc marks it as 'static'.
            method.func.attrs -= Attr::AttrStatic;
        }
        if method.flags.contains(MethodFlags::IS_CLOSURE_BODY) {
            // We want closure bodies to be 'instance' but hackc marks it as 'static'.
            method.func.attrs -= Attr::AttrStatic;
        }
    }

    // HHVM is okay with implicit 86pinit and 86sinit but we need to make them
    // explicit so we can put trivial initializers in them (done later in func
    // lowering).
    create_method_if_missing(&mut class, MethodName::_86pinit(), IsStatic::NonStatic);
    create_method_if_missing(&mut class, MethodName::_86sinit(), IsStatic::Static);

    if class.flags.contains(Attr::AttrIsClosureClass) {
        create_default_closure_constructor(&mut class);
    }

    // Turn class constants into properties.
    // TODO: Need to think about abstract constants. Maybe constants lookups
    // should really be function calls...
    for constant in class.constants.drain(..) {
        let Constant { name, value, attrs } = constant;
        // Mark the property as originally being a constant.
        let attributes = vec![Attribute {
            name: ClassName::intern(INFER_CONSTANT),
            arguments: vec![].into(),
        }];
        let type_info = if let Some(value) = value.as_ref().into_option() {
            TypeInfo::from_typed_value(value)
        } else {
            TypeInfo::empty()
        };
        let prop = Property {
            name: PropName::new(name.as_string_id()),
            flags: attrs | Attr::AttrStatic,
            attributes,
            visibility: Visibility::Public,
            initial_value: value.into(),
            type_info,
            doc_comment: Default::default(),
        };
        class.properties.push(prop);
    }

    let dict_constraint_name = Some(ir::intern(hhbc::BUILTIN_NAME_DICT));
    for tc in class.type_constants.drain(..) {
        let TypeConstant {
            name,
            initializer,
            is_abstract,
        } = tc;
        let arguments: Vec<TypedValue> = initializer
            .as_ref()
            .into_option()
            .and_then(compute_tc_attribute)
            .map(|s| TypedValue::String(ir::intern(s).as_bytes()))
            .into_iter()
            .collect();
        // Mark the property as originally being a type constant.
        let attributes = vec![Attribute {
            name: ClassName::intern(INFER_TYPE_CONSTANT),
            arguments: arguments.into(),
        }];
        let mut modifiers = TypeConstraintFlags::TypeConstant;
        if is_abstract {
            modifiers = modifiers | TypeConstraintFlags::Nullable;
        }
        let type_info = TypeInfo {
            user_type: None.into(),
            type_constraint: Constraint {
                name: dict_constraint_name.into(),
                flags: modifiers,
            },
        };
        let prop = Property {
            name: PropName::new(name),
            flags: Attr::AttrStatic,
            attributes,
            visibility: Visibility::Public,
            initial_value: initializer.into(),
            type_info,
            doc_comment: Default::default(),
        };
        class.properties.push(prop);
    }

    class
}

fn create_default_closure_constructor(class: &mut Class) {
    let name = MethodName::constructor();

    let func = FuncBuilder::build_func(|fb| {
        let loc = fb.add_loc(class.src_loc.clone());
        fb.func.loc_id = loc;

        // '$this' parameter is implied.

        for prop in &class.properties {
            fb.func.params.push(Param {
                name: prop.name.as_string_id(),
                is_variadic: false,
                is_inout: false,
                is_readonly: false,
                user_attributes: Vec::new(),
                ty: prop.type_info.clone(),
                default_value: None,
            });

            let lid = LocalId::Named(prop.name.as_string_id());
            let value = fb.emit(Instr::Hhbc(instr::Hhbc::CGetL(lid, loc)));
            MemberOpBuilder::base_h(loc).emit_set_m_pt(fb, prop.name, value);
        }

        let null = fb.emit_imm(Immediate::Null);
        fb.emit(Instr::ret(null, loc));
    });

    let method = Method {
        flags: MethodFlags::empty(),
        func,
        name,
        visibility: Visibility::Public,
    };
    class.methods.push(method);
}

fn create_method_if_missing(class: &mut Class, name: MethodName, is_static: IsStatic) {
    if class.methods.iter().any(|m| m.name == name) {
        return;
    }

    let func = FuncBuilder::build_func(|fb| {
        let loc = fb.add_loc(class.src_loc.clone());
        fb.func.loc_id = loc;
        fb.func.attrs = is_static.as_attr();
        let null = fb.emit_imm(Immediate::Null);
        fb.emit(Instr::ret(null, loc));
    });

    let method = Method {
        flags: MethodFlags::empty(),
        func,
        name,
        visibility: Visibility::Private,
    };
    class.methods.push(method);
}
