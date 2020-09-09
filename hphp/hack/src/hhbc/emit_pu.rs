// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhbc_string_utils_rust as string_utils;
use oxidized::aast_visitor::{visit_mut, AstParams, NodeMut, VisitorMut};
use oxidized::{aast, aast_defs, ast_defs, local_id};
use oxidized::{ast as Tast, pos::Pos};
use std::collections::HashMap;

// Remove specific Pocket Universes syntax in favor of plain Hack syntax.
pub fn translate(program: &mut Tast::Program) {
    for def in program.iter_mut() {
        update_def(def);
    }
}

fn pu_name_mangle(instance_name: &str, name: &str) -> String {
    format!("pu${}${}", instance_name, name)
}

type AccMap = HashMap<String, Vec<(String, Tast::Expr)>>;

// TODO(hrust) update most for / into_iter loops using fold/map patterns, like
// in gen_pu_accessors.
// vsiles: My plan is to do that once I can test this code and validate it is ok.

fn process_pumapping(
    atom_name: impl AsRef<str>,
    acc: &mut AccMap,
    pum_exprs: Vec<(Tast::Sid, Tast::Expr)>,
) {
    for expr in pum_exprs.into_iter() {
        let id = expr.0;
        let expr_value = expr.1;
        let ast_defs::Id(_, expr_name) = id;
        let mut empty = Vec::new();
        let list = acc.get_mut(&expr_name).unwrap_or(&mut empty);
        list.push((atom_name.as_ref().into(), expr_value));
        let new_list = list.to_vec();
        acc.insert(expr_name, new_list);
    }
}

fn process_class_enum(pu_members: Vec<Tast::PuMember>) -> AccMap {
    let mut info = AccMap::new();
    for Tast::PuMember { atom, exprs, .. } in pu_members.into_iter() {
        process_pumapping(&atom.1, &mut info, exprs);
    }
    info
}

fn simple_hint_(pos: Pos, name: impl Into<String>) -> aast_defs::Hint_ {
    let id = ast_defs::Id(pos, name.into());
    aast_defs::Hint_::Happly(id, Vec::new())
}

fn simple_hint(pos: Pos, name: impl Into<String>) -> aast_defs::Hint {
    aast_defs::Hint(pos.clone(), Box::new(simple_hint_(pos, name)))
}

fn apply_to_hint(pos: Pos, name: impl Into<String>, ty: aast_defs::Hint) -> aast_defs::Hint {
    let id = ast_defs::Id(pos.clone(), name.into());
    let happly = aast_defs::Hint_::Happly(id, vec![ty]);
    aast_defs::Hint(pos, Box::new(happly))
}

fn type_hint(pos: Pos, name: impl Into<String>) -> Tast::TypeHint {
    let ty = simple_hint(pos, name.into());
    Tast::TypeHint((), Some(ty))
}

fn create_mixed_type_hint(pos: Pos) -> Tast::TypeHint {
    type_hint(pos, "\\HH\\mixed")
}

// fn create_void_type_hint(pos: Pos) -> Tast::TypeHint {
//     type_hint(pos, "\\HH\\void".to_string())
// }

fn create_string_type_hint(pos: Pos) -> Tast::TypeHint {
    type_hint(pos, "\\HH\\string")
}

fn create_key_set_string_type_hint(pos: Pos) -> Tast::TypeHint {
    let hstring = simple_hint(pos.clone(), "\\HH\\string");
    let keyset_string = apply_to_hint(pos, "\\HH\\keyset", hstring);
    Tast::TypeHint((), Some(keyset_string))
}

/* Helper functions to generate Ast nodes */
fn id(pos: Pos, name: impl Into<String>) -> Tast::Expr {
    let eid = Tast::Expr_::mk_id(ast_defs::Id(pos.clone(), name.into()));
    Tast::Expr(pos, eid)
}

fn class_id(pos: Pos, name: impl Into<String>) -> Tast::ClassId {
    let cid = Tast::ClassId_::CIexpr(id(pos.clone(), name.into()));
    Tast::ClassId(pos, cid)
}

fn class_const(pos: Pos, cls: impl Into<String>, name: impl Into<String>) -> Tast::Expr {
    let expr = Tast::Expr_::mk_class_const(
        class_id(pos.clone(), cls.into()),
        (pos.clone(), name.into()),
    );
    Tast::Expr(pos, expr)
}

fn call(pos: Pos, caller: Tast::Expr, args: Vec<Tast::Expr>) -> Tast::Expr {
    let expr = Tast::Expr_::mk_call(caller, vec![], args, None);
    Tast::Expr(pos, expr)
}

fn lvar(pos: Pos, name: impl Into<String>) -> Tast::Expr {
    let local_id = local_id::make_unscoped(name.into());
    let lid = aast::Lid(pos.clone(), local_id);
    Tast::Expr(pos, Tast::Expr_::mk_lvar(lid))
}

fn str_(pos: Pos, name: String) -> Tast::Expr {
    let ename = Tast::Expr_::String(name.into());
    Tast::Expr(pos, ename)
}

// fn class_get(pos: Pos, cls: String, name: &str) -> Tast::Expr {
//     let gexpr = Tast::ClassGetExpr::CGstring((pos.clone(), format!("${}", name)));
//     let expr = Tast::Expr_::ClassGet(Box::new((class_id(pos.clone(), cls), gexpr)));
//     Tast::Expr(pos, expr)
// }

fn new_(pos: Pos, cls: impl Into<String>, args: Vec<Tast::Expr>) -> Tast::Expr {
    let expr = Tast::Expr_::mk_new(
        class_id(pos.clone(), cls.into()),
        vec![],
        args,
        None,
        pos.clone(),
    );
    Tast::Expr(pos, expr)
}

fn assign(pos: Pos, target: Tast::Expr, expr: Tast::Expr) -> Tast::Stmt {
    let binop = Tast::Expr_::mk_binop(ast_defs::Bop::Eq(None), target, expr);
    let expr = Tast::Expr(pos.clone(), binop);
    let stmt_ = Tast::Stmt_::mk_expr(expr);
    Tast::Stmt(pos, stmt_)
}

fn assign_lvar(pos: Pos, target: impl Into<String>, expr: Tast::Expr) -> Tast::Stmt {
    assign(pos.clone(), lvar(pos, target), expr)
}

fn obj_get(pos: Pos, var_name: impl Into<String>, method_name: impl Into<String>) -> Tast::Expr {
    let expr = Tast::Expr_::mk_obj_get(
        lvar(pos.clone(), var_name),
        id(pos.clone(), method_name),
        ast_defs::OgNullFlavor::OGNullthrows,
    );
    Tast::Expr(pos, expr)
}

fn return_(pos: Pos, expr: Tast::Expr) -> Tast::Stmt {
    let expr = Tast::Stmt_::mk_return(Some(expr));
    Tast::Stmt(pos, expr)
}

fn user_attribute(
    pos: Pos,
    name: impl Into<String>,
    params: Vec<Tast::Expr>,
) -> Tast::UserAttribute {
    Tast::UserAttribute {
        name: ast_defs::Id(pos, name.into()),
        params,
    }
}

fn memoize(pos: Pos) -> Tast::UserAttribute {
    user_attribute(pos, "__Memoize", vec![])
}

fn override_(pos: Pos) -> Tast::UserAttribute {
    user_attribute(pos, "__Override", vec![])
}

fn error_msg(cls: &str, field: &str, name: &str) -> String {
    format!("{}:@{}::{} unknown atom access: ", cls, field, name)
}

/* Generate a static accessor function from the pumapping information, eg:

   <<__Memoize>>
   static function pu$Field_name$Expr_name(string $atom) : mixed {
     switch ($atom) {
       case "A":
        return valA;
       case "B":
        return valB;
       ...
       default:
           raise \Exception "illegal..."
     }
    }

   If the class uses traits T1, .., Tn, then the default case calls the
   pu$Field_name$Expr_name method on traits T1, .., Tn, catching method
   not found errors via reflection (as explained below for the Members function).
   Given the semantics of trait method override, it is necessary to perform
   manually the search on the used traits.

   If the class extends a superclass, the raise statement is replaced with
   a call to parent::pu$Field_name$Expr_name.

   Remark: at compile time we cannot reliably detect if, whenever a class D
   extends a class C, a PU defined in the subclass D extends a PU defined in C
   or is self-contained.  In this case the accessor functions include the
   default call to parent.  Type checking ensures that the default case is
   not reachable if the PU was not inherited from the super-class.

*/
fn gen_pu_accessor(
    fun_name: String,
    pos: ast_defs::Pos,
    is_final: bool,
    extends: bool,
    uses: &[&str],
    infos: Vec<(String, Tast::Expr)>,
    error: String,
) -> Tast::Method_ {
    let var_name = "$atom".to_string();
    let var_atom = lvar(pos.clone(), &var_name);
    let do_case = |entry, init| {
        let entry = str_(pos.clone(), entry);
        let ret = return_(pos.clone(), init);
        Tast::Case::Case(entry.into(), vec![ret])
    };
    let mut cases = vec![];
    for info in infos.into_iter() {
        let (atom_name, value) = info;
        cases.push(do_case(atom_name, value.clone()));
    }
    let extends =
    /* returns a stmt_, to be used in the default branch */
    if extends {
        let parent_call = class_const(pos.clone(), "parent", &fun_name);
        let call = call(pos.clone(), parent_call, vec![var_atom.clone()]);
        return_(pos.clone(), call)
    } else {
        let msg_ = Tast::Expr_::mk_binop(
            ast_defs::Bop::Dot,
            str_(pos.clone(), error),
            var_atom.clone(),
        );
        let msg = Tast::Expr(pos.clone(), msg_);
        let new_exn = new_(pos.clone(), "\\Exception", vec![msg]);
        let throw = Tast::Stmt_::mk_throw(new_exn);
        Tast::Stmt(pos.clone(), throw)
    };
    let default = if uses.is_empty() {
        Tast::Case::Default(pos.clone(), vec![extends])
    } else {
        let uses_list: Vec<Tast::Afield> = uses
            .iter()
            .map(|x| {
                Tast::Afield::AFvalue(str_(pos.clone(), string_utils::strip_ns(x).to_string()))
            })
            .collect();
        let default_block = vec![
            /* $trait_classes = vec[MyTraitA::class, MyTraitB::class]; */
            assign_lvar(
                pos.clone(),
                "$trait_classes",
                Tast::Expr(
                    pos.clone(),
                    Tast::Expr_::mk_collection(
                        ast_defs::Id(pos.clone(), "vec".to_string()),
                        None,
                        uses_list,
                    ),
                ),
            ),
            /* foreach ($trait_classes as $trait_class) */
            Tast::Stmt(
                pos.clone(),
                Tast::Stmt_::mk_foreach(
                    lvar(pos.clone(), "$trait_classes"),
                    Tast::AsExpr::AsV(lvar(pos.clone(), "$trait_class")),
                    /* try */
                    vec![Tast::Stmt(
                        pos.clone(),
                        Tast::Stmt_::mk_try(
                            vec![
                                /* $class = new ReflectionClass($trait_class); */
                                assign_lvar(
                                    pos.clone(),
                                    "$class",
                                    new_(
                                        pos.clone(),
                                        "ReflectionClass",
                                        vec![lvar(pos.clone(), "$trait_class")],
                                    ),
                                ),
                                /*  $method = $class->getMethod('pu$E$value'); */
                                assign_lvar(
                                    pos.clone(),
                                    "$method",
                                    call(
                                        pos.clone(),
                                        obj_get(pos.clone(), "$class", "getMethod"),
                                        vec![str_(pos.clone(), fun_name.clone())],
                                    ),
                                ),
                                /* return $method->invoke(null,$atom); */
                                return_(
                                    pos.clone(),
                                    call(
                                        pos.clone(),
                                        obj_get(pos.clone(), "$method", "invoke"),
                                        vec![
                                            Tast::Expr(pos.clone(), Tast::Expr_::Null),
                                            var_atom.clone(),
                                        ],
                                    ),
                                ),
                            ],
                            /* catch (Exception $_) {} */
                            vec![Tast::Catch(
                                ast_defs::Id(pos.clone(), "Exception".to_string()),
                                aast::Lid(pos.clone(), local_id::make_unscoped("$_".to_string())),
                                vec![],
                            )],
                            vec![],
                        ),
                    )],
                ),
            ),
            /* parent::pu$E$value($atom), or raise an exception */
            extends,
        ];
        Tast::Case::Default(pos.clone(), default_block)
    };
    cases.push(default);
    let body = {
        let switch = Tast::Stmt_::mk_switch(var_atom, cases);
        Tast::FuncBody {
            ast: vec![Tast::Stmt(pos.clone(), switch)],
            annotation: (),
        }
    };
    Tast::Method_ {
        span: pos.clone(),
        annotation: (),
        final_: is_final,
        abstract_: false,
        static_: true,
        visibility: aast_defs::Visibility::Public,
        name: ast_defs::Id(pos.clone(), fun_name),
        tparams: vec![],
        where_constraints: vec![],
        variadic: Tast::FunVariadicity::FVnonVariadic,
        params: vec![Tast::FunParam {
            annotation: pos.clone(),
            type_hint: create_string_type_hint(pos.clone()),
            is_variadic: false,
            pos: pos.clone(),
            name: var_name,
            expr: None,
            callconv: None,
            user_attributes: vec![],
            visibility: None,
        }],
        cap: create_mixed_type_hint(pos.clone()),
        unsafe_cap: create_mixed_type_hint(pos.clone()),
        body,
        fun_kind: ast_defs::FunKind::FSync,
        user_attributes: vec![memoize(pos.clone())],
        ret: create_mixed_type_hint(pos),
        external: false,
        doc_comment: None,
    }
}

/* Generate a helper/debug function called Members which is a keyset
   of all the atoms declared in a ClassEnum.

   All PU classes get a new private static method called `Members` which
   returns a keyset<string> of all the instance names of a universe.

   If a class doesn't extend any other, the method directly
   returns the list of the instance names, available locally.

   If a class extends another one, we might need to look into it, so
   Members will do the recursive call, to correctly get them all.
   Since this can be expensive, we Memoize the function so it is done only
   once.
*/

/* As for accessors, we do not have a reliable way to detect, whenever a
   PU is defined in a subclass, if the PU is inherited from the superclass,
   or if instances are defined in traits being used.  We thus systematically
   perform a call to Members() in the used traits and in the parent class to
   check if there are instances of the PU defined in the trait/superclass.
   As the Members method might not exist in the trait/superclass, we encapsulate
   the call using reflection and catch the eventual exception.
   Reflection can be slow, but Memoization ensures that the class hierarchy
   is explored only once.

   The general shape is:

  <<__Memoize, __Override>>
  public static function pu$E$Members() : keyset<string> {
    $result = keyset[ .. strings based on local PU instances ...];
    // exploring used traits
    $traits = vec[ ... used traits ...];
    foreach ($traits as $traits_class) {
      try {
        $class = new ReflectionClass($traits_class);
        // might throw if the method is not in the parent class
        $method = $class->getMethod('pu$E$Members');
        // method is here, call it
        $parent_members = $method->invoke(null);
        foreach ($parent_members as $p) {
          $result[] = $p;
        }
      } catch (ReflectionException $_) {
      }
    }
    // exploring the parent
    try {
      $class = new ReflectionClass(parent::class);
      // might throw if the method is not in the parent class
      $method = $class->getMethod('pu$E$Members');
      // method is here, call it
      $parent_members = $method->invoke(null);
      foreach ($parent_members as $p) {
        $result[] = $p;
      }
    } catch (ReflectionException $_) {
      // not the right method: just use local info
    }
   return $result;
  }
*/
fn gen_members_search(
    instance_name: &str,
    pos: Pos,
    extends: bool,
    uses: &[&str],
    pu_members: Vec<Tast::PuMember>,
) -> Tast::Method_ {
    let m_members = pu_name_mangle(instance_name, "Members");
    let members = pu_members
        .into_iter()
        .map(|member: Tast::PuMember| {
            let atom = member.atom.1;
            Tast::Afield::AFvalue(str_(pos.clone(), atom))
        })
        .collect();
    let mems_ = Tast::Expr_::mk_collection(
        ast_defs::Id(pos.clone(), "keyset".to_string()),
        None,
        members,
    );
    let mems = Tast::Expr(pos.clone(), mems_);
    let uses_list: Vec<Tast::Afield> = uses
        .iter()
        .map(|x| Tast::Afield::AFvalue(str_(pos.clone(), string_utils::strip_ns(x).to_string())))
        .collect();
    let update_members_from = |expr: Tast::Expr| {
        vec![Tast::Stmt(
            pos.clone(),
            /* try { */
            Tast::Stmt_::mk_try(
                vec![
                    /* $class = new ReflectionClass(parent::class / $traits_class) */
                    assign_lvar(
                        pos.clone(),
                        "$class",
                        new_(pos.clone(), "ReflectionClass", vec![expr]),
                    ),
                    /* $method = $class->getMethod('pu$E$Members'); */
                    assign_lvar(
                        pos.clone(),
                        "$method",
                        call(
                            pos.clone(),
                            obj_get(pos.clone(), "$class", "getMethod"),
                            vec![str_(pos.clone(), m_members.clone())],
                        ),
                    ),
                    /* $parent_members = $method->invoke(null); */
                    assign_lvar(
                        pos.clone(),
                        "$parent_members",
                        call(
                            pos.clone(),
                            obj_get(pos.clone(), "$method", "invoke"),
                            vec![Tast::Expr(pos.clone(), Tast::Expr_::Null)],
                        ),
                    ),
                    /* foreach ($parent_members as $p) { $result[] = $p; } */
                    Tast::Stmt(
                        pos.clone(),
                        Tast::Stmt_::mk_foreach(
                            lvar(pos.clone(), "$parent_members"),
                            Tast::AsExpr::AsV(lvar(pos.clone(), "$p")),
                            vec![assign(
                                pos.clone(),
                                Tast::Expr(
                                    pos.clone(),
                                    Tast::Expr_::mk_array_get(lvar(pos.clone(), "$result"), None),
                                ),
                                lvar(pos.clone(), "$p"),
                            )],
                        ),
                    ),
                ],
                /* } catch (ReflectionException $_) { } */
                vec![Tast::Catch(
                    ast_defs::Id(pos.clone(), "ReflectionException".to_string()),
                    aast::Lid(pos.clone(), local_id::make_unscoped("$_".to_string())),
                    vec![],
                )],
                vec![],
            ),
        )]
    };
    let members_traits = if uses.is_empty() {
        vec![]
    } else {
        vec![
            /* $traits_classes = vec[ ... used traits ...] */
            assign_lvar(
                pos.clone(),
                "$traits_classes",
                Tast::Expr(
                    pos.clone(),
                    Tast::Expr_::mk_collection(
                        ast_defs::Id(pos.clone(), "vec".to_string()),
                        None,
                        uses_list,
                    ),
                ),
            ),
            /* foreach ($traits_classes as $traits_class) */
            Tast::Stmt(
                pos.clone(),
                Tast::Stmt_::mk_foreach(
                    lvar(pos.clone(), "$traits_classes"),
                    Tast::AsExpr::AsV(lvar(pos.clone(), "$traits_class")),
                    update_members_from(lvar(pos.clone(), "$traits_class")),
                ),
            ),
        ]
    };
    let members_parent = if !extends {
        vec![]
    } else {
        update_members_from(class_const(pos.clone(), "parent", "class"))
    };
    let body = {
        let mut ast = vec![
            /* $result = keyset[ ... names of local instances ... ] */
            assign_lvar(pos.clone(), "$result", mems),
        ];
        ast.extend(members_traits);
        ast.extend(members_parent);
        ast.extend(vec![
            /* return $result; */
            return_(pos.clone(), lvar(pos.clone(), "$result")),
        ]);
        Tast::FuncBody {
            ast,
            annotation: (),
        }
    };
    Tast::Method_ {
        span: pos.clone(),
        annotation: (),
        final_: false,
        abstract_: false,
        static_: true,
        visibility: aast_defs::Visibility::Public,
        name: ast_defs::Id(pos.clone(), m_members),
        tparams: vec![],
        where_constraints: vec![],
        variadic: Tast::FunVariadicity::FVnonVariadic,
        params: vec![],
        cap: create_mixed_type_hint(pos.clone()),
        unsafe_cap: create_mixed_type_hint(pos.clone()),
        body,
        fun_kind: ast_defs::FunKind::FSync,
        user_attributes: vec![memoize(pos.clone()), override_(pos.clone())],
        ret: create_key_set_string_type_hint(pos),
        external: false,
        doc_comment: None,
    }
}

/*
If the class doesn't extends another one, everything is available locally
 <<__Memoize>>
 public static function pu$E$Members() : keyset<string> {
   return keyset[ .. strings based on local PU instances ...];
  }
*/
fn gen_members_no_search(
    instance_name: &str,
    pos: Pos,
    pu_members: Vec<Tast::PuMember>,
) -> Tast::Method_ {
    let m_members = pu_name_mangle(instance_name, "Members");
    let members = pu_members
        .into_iter()
        .map(|member: Tast::PuMember| {
            let atom = member.atom.1;
            Tast::Afield::AFvalue(str_(pos.clone(), atom))
        })
        .collect();
    let mems_ = Tast::Expr_::mk_collection(
        ast_defs::Id(pos.clone(), "keyset".to_string()),
        None,
        members,
    );
    let mems = Tast::Expr(pos.clone(), mems_);
    let body = {
        let ast = vec![return_(pos.clone(), mems)];
        Tast::FuncBody {
            ast,
            annotation: (),
        }
    };
    Tast::Method_ {
        span: pos.clone(),
        annotation: (),
        final_: false,
        abstract_: false,
        static_: true,
        visibility: aast_defs::Visibility::Public,
        name: ast_defs::Id(pos.clone(), m_members),
        tparams: vec![],
        where_constraints: vec![],
        variadic: Tast::FunVariadicity::FVnonVariadic,
        params: vec![],
        cap: create_mixed_type_hint(pos.clone()),
        unsafe_cap: create_mixed_type_hint(pos.clone()),
        body,
        fun_kind: ast_defs::FunKind::FSync,
        user_attributes: vec![memoize(pos.clone())],
        ret: create_key_set_string_type_hint(pos),
        external: false,
        doc_comment: None,
    }
}

/* Returns the generated methods (accessors + Members) */
fn gen_pu_methods(
    class_name: &str,
    extends: bool,
    uses: &[&str],
    instance: Tast::PuEnum,
) -> Vec<Tast::Method_> {
    let is_final = instance.is_final;
    let pu_members = instance.members;
    let ast_defs::Id(pos, instance_name) = instance.name.clone();
    let infos = process_class_enum(pu_members.clone());
    let m_members = if extends || !uses.is_empty() {
        gen_members_search(&instance_name, pos.clone(), extends, uses, pu_members)
    } else {
        gen_members_no_search(&instance_name, pos.clone(), pu_members)
    };
    let mut acc = std::iter::once(m_members)
        .chain(infos.into_iter().map(|(key, info)| {
            let fun_name = pu_name_mangle(&instance_name, &key);
            let error = error_msg(&class_name, &instance_name, &key);
            gen_pu_accessor(fun_name, pos.clone(), is_final, extends, uses, info, error)
        }))
        .collect::<Vec<_>>();

    // TODO(hrust)(vsiles)
    // I'm keeping this around until I can test it to understand what's happening. Will remove
    // the loop version once tests are done
    // let mut acc = vec![];
    // for (key, info) in infos.iter() {
    //     let fun_name = gen_fun_name(field_name, key);
    //     let error = error_msg(class_name, field_name, key);
    //     let accessor = gen_pu_accessor(fun_name, pos.clone(), is_final, extends, info, error);
    //     acc.push(accessor);
    // }
    // let fun_members = gen_members(field);
    // acc.push(fun_members);
    // acc.reverse();

    // TODO(hrust) this sort is not necessary, it only helps for testing parity between rust/ocaml.
    // Remove it after rust emitted shipped
    acc.sort_by(|a, b| a.name.1.cmp(&b.name.1));
    acc
}

struct EraseBodyVisitor {}
struct Ctx {}

impl<'ast> VisitorMut<'ast> for EraseBodyVisitor {
    type P = AstParams<Ctx, ()>;

    fn object(&mut self) -> &mut dyn VisitorMut<'ast, P = Self::P> {
        self
    }

    fn visit_expr_(&mut self, c: &mut Ctx, e: &mut Tast::Expr_) -> Result<(), ()> {
        match e {
            Tast::Expr_::PUAtom(atom) => Ok(*e = Tast::Expr_::String(atom.clone().into())),
            Tast::Expr_::PUIdentifier(pui) => {
                let class_id = pui.0.clone();
                let pfield = &pui.1;
                let name = &(pui.2).1;
                let (pos, field) = pfield;
                let fun_name = (pos.clone(), pu_name_mangle(&field, &name));
                Ok(*e = Tast::Expr_::mk_class_const(class_id, fun_name))
            }
            _ => e.recurse(c, self.object()),
        }
    }

    fn visit_hint(&mut self, c: &mut Ctx, h: &mut Tast::Hint) -> Result<(), ()> {
        let aast_defs::Hint(p, h) = h;
        match h.as_ref() {
            Tast::Hint_::HpuAccess(_, _) => {
                Ok(*h = Box::new(simple_hint_(p.clone(), "\\HH\\mixed")))
            }
            Tast::Hint_::Hprim(prim) => Ok(match prim {
                aast_defs::Tprim::Tatom(_) => {
                    *h = Box::new(Tast::Hint_::Hprim(aast_defs::Tprim::Tstring))
                }
                _ => {}
            }),
            _ => h.recurse(c, self.object()),
        }
    }
}

fn erase_stmt(stmt: &mut Tast::Stmt) {
    visit_mut(&mut EraseBodyVisitor {}, &mut Ctx {}, stmt).unwrap();
}

fn erase_fun(f: &mut Tast::Fun_) {
    visit_mut(&mut EraseBodyVisitor {}, &mut Ctx {}, f).unwrap();
}

fn process_pufields(
    class_name: &str,
    extends: bool,
    uses: &[&str],
    pu_enums: Vec<Tast::PuEnum>,
) -> Vec<Tast::Method_> {
    pu_enums
        .into_iter()
        .flat_map(|pu_enum| gen_pu_methods(class_name, extends, uses, pu_enum))
        .collect()
}

fn trait_name_from_hint(h: &aast::TraitHint) -> Option<&str> {
    let aast_defs::Hint(_, hint) = h;
    match hint.as_ref() {
        Tast::Hint_::Happly(sid, _) => {
            let ast_defs::Id(_, id) = sid;
            Some(id)
        }
        _ => None,
    }
}
fn update_class(c: &mut Tast::Class_) {
    let class_name = &c.name.1;
    let extends = !c.extends.is_empty();
    let pu_enums = std::mem::replace(&mut c.pu_enums, vec![]);
    let uses: Vec<&str> = c.uses.iter().filter_map(trait_name_from_hint).collect();
    let mut pu_methods = process_pufields(class_name, extends, &uses, pu_enums);
    visit_mut(&mut EraseBodyVisitor {}, &mut Ctx {}, c).unwrap();
    c.methods.append(&mut pu_methods);
}

fn update_def(d: &mut Tast::Def) {
    match d {
        Tast::Def::Class(c) => update_class(c),
        Tast::Def::Stmt(s) => erase_stmt(s),
        Tast::Def::Fun(f) => erase_fun(f),
        _ => {}
    }
}
