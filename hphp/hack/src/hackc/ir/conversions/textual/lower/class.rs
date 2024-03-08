use std::sync::Arc;

use ir::instr;
// use ir::print::print;
use ir::Attr;
use ir::Attribute;
use ir::BaseType;
use ir::Class;
use ir::ClassName;
use ir::Constant;
use ir::EnforceableType;
use ir::FuncBuilder;
use ir::HackConstant;
use ir::Instr;
use ir::LocalId;
use ir::MemberOpBuilder;
use ir::Method;
use ir::MethodFlags;
use ir::MethodId;
use ir::Param;
use ir::PropId;
use ir::Property;
use ir::StringInterner;
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

fn typed_value_into_string(tv: &TypedValue, strings: &Arc<StringInterner>) -> Option<String> {
    let ostr = tv
        .get_string()
        .map(|sid| strings.lookup_bstr(sid).to_string());
    trace!("tv {:?} -> {:?}", tv, ostr);
    ostr
}

fn typed_value_into_strings(tv: &TypedValue, strings: &Arc<StringInterner>) -> Option<Vec<String>> {
    let ovs = if let TypedValue::Vec(tvs) = tv {
        tvs.iter()
            .map(|tv| typed_value_into_string(tv, strings))
            .collect()
    } else {
        None
    };
    trace!("tvs {:?} -> {:?}", tv, ovs);
    ovs
}

fn compute_tc_attribute(typed_value: &TypedValue, strings: &Arc<StringInterner>) -> Option<String> {
    match typed_value {
        TypedValue::Dict(dict) => {
            let kind_key = ir::ArrayKey::String(strings.intern_str("kind"));
            dict.get(&kind_key)
                .and_then(|tv| tv.get_int())
                .and_then(|i| {
                    // TODO(dpichardie) Incomplete support for type constant definitions.
                    // One should add T_int, T_bool, ...
                    let unresolved: i64 = ir::TypeStructureKind::T_unresolved.into();
                    let type_access: i64 = ir::TypeStructureKind::T_typeaccess.into();
                    if i == unresolved {
                        let class_name = ir::ArrayKey::String(strings.intern_str("classname"));
                        dict.get(&class_name)
                            .and_then(|cn| typed_value_into_string(cn, strings))
                    } else if i == type_access {
                        let root_name = ir::ArrayKey::String(strings.intern_str("root_name"));
                        let access_list = ir::ArrayKey::String(strings.intern_str("access_list"));
                        let root_name = dict
                            .get(&root_name)
                            .and_then(|rn| typed_value_into_string(rn, strings));
                        let access_list = dict
                            .get(&access_list)
                            .and_then(|al| typed_value_into_strings(al, strings));
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

pub(crate) fn lower_class(mut class: Class, strings: Arc<StringInterner>) -> Class {
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
        let name = PropId::new(strings.intern_str(THIS_AS_PROPERTY));
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
        if method.name.is_86pinit(&strings) {
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
    create_method_if_missing(
        &mut class,
        MethodId::_86pinit(&strings),
        IsStatic::NonStatic,
        Arc::clone(&strings),
    );
    create_method_if_missing(
        &mut class,
        MethodId::_86sinit(&strings),
        IsStatic::Static,
        Arc::clone(&strings),
    );

    if class.flags.contains(Attr::AttrIsClosureClass) {
        create_default_closure_constructor(&mut class, Arc::clone(&strings));
    }

    // Turn class constants into properties.
    // TODO: Need to think about abstract constants. Maybe constants lookups
    // should really be function calls...
    for constant in class.constants.drain(..) {
        let HackConstant { name, value, attrs } = constant;
        // Mark the property as originally being a constant.
        let attributes = vec![Attribute {
            name: ClassName::intern(INFER_CONSTANT),
            arguments: Vec::new(),
        }];
        let type_info = if let Some(value) = value.as_ref() {
            value.type_info()
        } else {
            TypeInfo::empty()
        };
        let prop = Property {
            name: PropId::new(name.as_bytes_id()),
            flags: attrs | Attr::AttrStatic,
            attributes,
            visibility: Visibility::Public,
            initial_value: value,
            type_info,
            doc_comment: Default::default(),
        };
        class.properties.push(prop);
    }

    for tc in class.type_constants.drain(..) {
        let TypeConstant {
            name,
            initializer,
            is_abstract,
        } = tc;
        let name = PropId::from_bytes(name.as_str().as_bytes(), &strings);
        let arguments: Vec<TypedValue> = initializer
            .as_ref()
            .and_then(|init| compute_tc_attribute(init, &strings))
            .map(|s| {
                let sid = strings.intern_str(s.clone());
                TypedValue::String(sid)
            })
            .into_iter()
            .collect();
        // Mark the property as originally being a type constant.
        let attributes = vec![Attribute {
            name: ClassName::intern(INFER_TYPE_CONSTANT),
            arguments,
        }];
        let mut modifiers = TypeConstraintFlags::TypeConstant;
        if is_abstract {
            modifiers = modifiers | TypeConstraintFlags::Nullable;
        }
        let type_info = TypeInfo {
            user_type: None,
            enforced: EnforceableType {
                ty: BaseType::Dict,
                modifiers,
            },
        };
        let prop = Property {
            name,
            flags: Attr::AttrStatic,
            attributes,
            visibility: Visibility::Public,
            initial_value: initializer,
            type_info,
            doc_comment: Default::default(),
        };
        class.properties.push(prop);
    }

    class
}

fn create_default_closure_constructor(class: &mut Class, strings: Arc<StringInterner>) {
    let name = MethodId::constructor(&strings);

    let func = FuncBuilder::build_func(Arc::clone(&strings), |fb| {
        let loc = fb.add_loc(class.src_loc.clone());
        fb.func.loc_id = loc;

        // '$this' parameter is implied.

        for prop in &class.properties {
            fb.func.params.push(Param {
                name: prop.name.id,
                is_variadic: false,
                is_inout: false,
                is_readonly: false,
                user_attributes: Vec::new(),
                ty: prop.type_info.clone(),
                default_value: None,
            });

            let lid = LocalId::Named(prop.name.id);
            let value = fb.emit(Instr::Hhbc(instr::Hhbc::CGetL(lid, loc)));
            MemberOpBuilder::base_h(loc).emit_set_m_pt(fb, prop.name, value);
        }

        let null = fb.emit_constant(Constant::Null);
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

fn create_method_if_missing(
    class: &mut Class,
    name: MethodId,
    is_static: IsStatic,
    strings: Arc<StringInterner>,
) {
    if class.methods.iter().any(|m| m.name == name) {
        return;
    }

    let func = FuncBuilder::build_func(strings, |fb| {
        let loc = fb.add_loc(class.src_loc.clone());
        fb.func.loc_id = loc;
        fb.func.attrs = is_static.as_attr();
        let null = fb.emit_constant(Constant::Null);
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
