// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

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

fn gen_fun_name(field: &str, name: &str) -> String {
    format!("{}##{}", field, name)
}

type AccMap = HashMap<String, Vec<(String, Tast::Expr)>>;

// TODO(hrust) update most for / into_iter loops using fold/map patterns, like
// in gen_pu_accessors.
// vsiles: My plan is to do that once I can test this code and validate it is ok.

fn process_pumapping(atom_name: String, acc: &mut AccMap, pu_member: Tast::PuMember) {
    let pum_exprs = pu_member.exprs;
    for expr in pum_exprs.into_iter() {
        let id = expr.0;
        let expr_value = expr.1;
        let ast_defs::Id(_, expr_name) = id;
        let mut empty = Vec::new();
        let list = acc.get_mut(&expr_name).unwrap_or(&mut empty);
        list.push((atom_name.clone(), expr_value));
        let new_list = list.to_vec();
        acc.insert(expr_name, new_list);
    }
}

fn process_class_enum(pu_members: Vec<Tast::PuMember>) -> AccMap {
    let mut info = AccMap::new();
    for member in pu_members.into_iter() {
        let ast_defs::Id(_, pum_atom) = member.atom.clone();
        process_pumapping(pum_atom, &mut info, member);
    }
    info
}

fn simple_typ(pos: Pos, name: String) -> aast_defs::Hint {
    let id = ast_defs::Id(pos.clone(), name);
    let happly = aast_defs::Hint_::Happly(id, Vec::new());
    aast_defs::Hint(pos, Box::new(happly))
}

fn error_msg(cls: &str, field: &str, name: &str) -> String {
    format!("{}::{}::{} unknown atom access: ", cls, field, name)
}

fn build_string(pos: Pos, name: String) -> Tast::Expr {
    let ename = Tast::Expr_::String(name);
    Tast::Expr(pos, ename)
}

fn build_id(pos: Pos, name: String) -> Tast::Expr {
    let eid = Tast::Expr_::Id(Box::new(ast_defs::Id(pos.clone(), name)));
    Tast::Expr(pos, eid)
}

fn build_lvar(pos: Pos, name: String) -> Tast::Expr {
    let local_id = local_id::make_unscoped(name);
    let lid = aast::Lid(pos.clone(), local_id);
    Tast::Expr(pos.clone(), Tast::Expr_::Lvar(Box::new(lid)))
}

fn gen_pu_accessor(
    fun_name: String,
    pos: ast_defs::Pos,
    is_final: bool,
    extends: bool,
    infos: &Vec<(String, Tast::Expr)>,
    error: String,
) -> Tast::Method_ {
    let var_atom = build_lvar(pos.clone(), "$atom".to_string());
    let do_case = |entry, init| {
        let entry = build_string(pos.clone(), entry);
        let ret = Tast::Stmt_::Return(Box::new(Some(init)));
        Tast::Case::Case(entry, vec![Tast::Stmt(pos.clone(), ret)])
    };
    let mut cases = vec![];
    for info in infos.into_iter() {
        let (atom_name, value) = info;
        cases.push(do_case(atom_name.to_string(), value.clone()));
    }
    let default = if extends {
        let class_id = Tast::ClassId(
            pos.clone(),
            Tast::ClassId_::CIexpr(build_id(pos.clone(), "parent".to_string())),
        );
        let parent_call =
            Tast::Expr_::ClassConst(Box::new((class_id, (pos.clone(), fun_name.clone()))));
        let call = Tast::Expr_::Call(Box::new((
            aast_defs::CallType::Cnormal,
            Tast::Expr(pos.clone(), parent_call),
            vec![],
            vec![var_atom.clone()],
            None,
        )));
        let ret = Tast::Stmt_::Return(Box::new(Some(Tast::Expr(pos.clone(), call))));
        Tast::Case::Default(pos.clone(), vec![Tast::Stmt(pos.clone(), ret)])
    } else {
        let msg = Tast::Expr_::Binop(Box::new((
            ast_defs::Bop::Dot,
            build_string(pos.clone(), error),
            var_atom.clone(),
        )));
        let class_id = Tast::ClassId(
            pos.clone(),
            Tast::ClassId_::CIexpr(build_id(pos.clone(), "\\Expection".to_string())),
        );
        let new = Tast::Expr_::New(Box::new((
            class_id,
            vec![],
            vec![Tast::Expr(pos.clone(), msg)],
            None,
            pos.clone(),
        )));
        let throw = Tast::Stmt_::Throw(Box::new(Tast::Expr(pos.clone(), new)));
        Tast::Case::Default(pos.clone(), vec![Tast::Stmt(pos.clone(), throw)])
    };
    cases.push(default);
    let body = {
        let switch = Tast::Stmt_::Switch(Box::new((var_atom, cases)));
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
            type_hint: Tast::TypeHint(
                (),
                Some(simple_typ(pos.clone(), "\\HH\\string".to_string())),
            ),
            is_variadic: false,
            pos: pos.clone(),
            name: "$atom".to_string(),
            expr: None,
            callconv: None,
            user_attributes: vec![],
            visibility: None,
        }],
        body,
        fun_kind: ast_defs::FunKind::FSync,
        user_attributes: vec![],
        ret: Tast::TypeHint((), Some(simple_typ(pos, "\\HH\\mixed".to_string()))),
        external: false,
        doc_comment: None,
    }
}

fn gen_members(fields: Tast::PuEnum) -> Tast::Method_ {
    let ast_defs::Id(pos, field) = fields.name;
    let pu_members = fields.members;
    let members = pu_members
        .into_iter()
        .map(|member: Tast::PuMember| {
            let atom = member.atom.1;
            Tast::Afield::AFvalue(build_string(pos.clone(), atom))
        })
        .collect();
    let body = {
        let mems = Tast::Expr_::Collection(Box::new((
            ast_defs::Id(pos.clone(), "vec".to_string()),
            None,
            members,
        )));
        let binop = Tast::Expr_::Binop(Box::new((
            ast_defs::Bop::Eq(None),
            build_lvar(pos.clone(), "$mems".to_string()),
            Tast::Expr(pos.clone(), mems),
        )));
        let binop = Tast::Stmt_::Expr(Box::new(Tast::Expr(pos.clone(), binop)));
        let binop = Tast::Stmt(pos.clone(), binop);
        let ret = Tast::Stmt_::Return(Box::new(Some(build_lvar(pos.clone(), "$mems".to_string()))));
        let ret = Tast::Stmt(pos.clone(), ret);
        Tast::FuncBody {
            ast: vec![binop, ret],
            annotation: (),
        }
    };
    Tast::Method_ {
        span: pos.clone(),
        annotation: (),
        final_: fields.is_final,
        abstract_: false,
        static_: true,
        visibility: aast_defs::Visibility::Public,
        name: ast_defs::Id(pos.clone(), gen_fun_name(&field, "Members")),
        tparams: vec![],
        where_constraints: vec![],
        variadic: Tast::FunVariadicity::FVnonVariadic,
        params: vec![],
        body,
        fun_kind: ast_defs::FunKind::FSync,
        user_attributes: vec![],
        ret: Tast::TypeHint((), Some(simple_typ(pos, "\\HH\\mixed".to_string()))),
        external: false,
        doc_comment: None,
    }
}

fn gen_pu_accessors(class_name: &str, extends: bool, field: Tast::PuEnum) -> Vec<Tast::Method_> {
    let ast_defs::Id(pos, field_name) = field.name.clone();
    let is_final = field.is_final;
    let infos = process_class_enum(field.members.clone());
    let fun_members = gen_members(field);
    let acc = infos
        .into_iter()
        .map(|(key, info)| {
            let fun_name = gen_fun_name(&field_name, &key);
            let error = error_msg(&class_name, &field_name, &key);
            gen_pu_accessor(fun_name, pos.clone(), is_final, extends, &info, error)
        })
        .chain(std::iter::once(fun_members))
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
    acc
}

struct EraseBodyVisitor {}
struct Ctx {}

impl VisitorMut for EraseBodyVisitor {
    type P = AstParams<Ctx, ()>;

    fn object(&mut self) -> &mut dyn VisitorMut<P = Self::P> {
        self
    }

    fn visit_expr_(&mut self, c: &mut Ctx, e: &mut Tast::Expr_) -> Result<(), ()> {
        match e {
            Tast::Expr_::PUAtom(atom) => Ok(*e = Tast::Expr_::String(atom.to_string())),
            Tast::Expr_::PUIdentifier(pui) => {
                let class_id = pui.0.clone();
                let pfield = &pui.1;
                let name = &(pui.2).1;
                let (pos, field) = pfield;
                let fun_name = (pos.clone(), gen_fun_name(field, name));
                Ok(*e = Tast::Expr_::ClassConst(Box::new((class_id, fun_name))))
            }
            _ => e.recurse(c, self.object()),
        }
    }

    fn visit_hint_(&mut self, c: &mut Ctx, h: &mut Tast::Hint_) -> Result<(), ()> {
        match h {
            Tast::Hint_::HpuAccess(_, _, _) => Ok(*h = Tast::Hint_::Hmixed),
            Tast::Hint_::Hprim(prim) => Ok(match prim {
                aast_defs::Tprim::Tatom(_) => *h = Tast::Hint_::Hprim(aast_defs::Tprim::Tstring),
                _ => (),
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
    pu_enums: Vec<Tast::PuEnum>,
) -> Vec<Tast::Method_> {
    pu_enums
        .into_iter()
        .flat_map(|pu_enum| gen_pu_accessors(class_name, extends, pu_enum))
        .collect()
}

fn update_class(c: &mut Tast::Class_) {
    let class_name = &c.name.1;
    let extends = c.extends.is_empty();
    let pu_enums = std::mem::replace(&mut c.pu_enums, vec![]);
    let mut pu_methods = process_pufields(class_name, extends, pu_enums);
    visit_mut(&mut EraseBodyVisitor {}, &mut Ctx {}, c).unwrap();
    c.methods.append(&mut pu_methods);
}

fn update_def(d: &mut Tast::Def) {
    match d {
        Tast::Def::Class(c) => update_class(c),
        Tast::Def::Stmt(s) => erase_stmt(s),
        Tast::Def::Fun(f) => erase_fun(f),
        _ => (),
    }
}
