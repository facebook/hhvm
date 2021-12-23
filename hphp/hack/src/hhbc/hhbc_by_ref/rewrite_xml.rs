use env::emitter::Emitter;
use hhbc_by_ref_hhbc_id::{class, Id};
use instruction_sequence::Result;
use naming_special_names_rust::pseudo_consts;
use oxidized::{
    aast_visitor::{visit_mut, AstParams, NodeMut, VisitorMut},
    ast, ast_defs,
    pos::Pos,
};

struct RewriteXmlVisitor<'emitter, 'arena, 'decl> {
    phantom: std::marker::PhantomData<(&'emitter &'arena (), &'emitter &'decl ())>,
}

struct Ctx<'emitter, 'arena, 'decl> {
    emitter: &'emitter mut Emitter<'arena, 'decl>,
}

impl<'ast, 'arena, 'emitter, 'decl> VisitorMut<'ast>
    for RewriteXmlVisitor<'emitter, 'arena, 'decl>
{
    type P = AstParams<Ctx<'emitter, 'arena, 'decl>, instruction_sequence::Error>;

    fn object(&mut self) -> &mut dyn VisitorMut<'ast, P = Self::P> {
        self
    }

    fn visit_expr(
        &mut self,
        c: &mut Ctx<'emitter, 'arena, 'decl>,
        e: &'ast mut ast::Expr,
    ) -> Result<()> {
        let ast::Expr(_, pos, expr) = e;
        let emitter = &mut c.emitter;
        if let ast::Expr_::Xml(cs) = expr {
            *e = rewrite_xml_(emitter, pos, cs.as_ref().clone())?;
        }
        e.recurse(c, self.object())?;
        Ok(())
    }
}

pub fn rewrite_xml<'p, 'arena, 'emitter, 'decl>(
    emitter: &'emitter mut Emitter<'arena, 'decl>,
    prog: &'p mut ast::Program,
) -> Result<()> {
    let mut xml_visitor = RewriteXmlVisitor {
        phantom: std::marker::PhantomData,
    };
    let mut c: Ctx<'emitter, 'arena, 'decl> = Ctx { emitter };

    visit_mut(&mut xml_visitor, &mut c, prog)
}

fn rewrite_xml_<'arena, 'decl>(
    e: &mut Emitter<'arena, 'decl>,
    pos: &Pos,
    (id, attributes, children): (ast::Sid, Vec<ast::XhpAttribute>, Vec<ast::Expr>),
) -> Result<ast::Expr> {
    use ast::{ClassId, ClassId_, Expr as E, Expr_ as E_, XhpAttribute};
    use ast_defs::{Id, ShapeFieldName as SF};

    let (_, attributes) =
        attributes
            .into_iter()
            .fold((0, vec![]), |(mut spread_id, mut attrs), attr| {
                match attr {
                    XhpAttribute::XhpSimple(xhp_simple) => {
                        let (pos, name) = xhp_simple.name;
                        attrs.push((SF::SFlitStr((pos, name.into())), xhp_simple.expr));
                    }
                    XhpAttribute::XhpSpread(expr) => {
                        attrs.push((
                            SF::SFlitStr((
                                expr.1.clone(),
                                format!("...${}", spread_id.to_string()).into(),
                            )),
                            expr,
                        ));
                        spread_id += 1;
                    }
                }
                (spread_id, attrs)
            });
    let attribute_map = E((), pos.clone(), E_::mk_shape(attributes));
    let children_vec = E((), pos.clone(), E_::mk_varray(None, children));
    let filename = E(
        (),
        pos.clone(),
        E_::mk_id(Id(pos.clone(), pseudo_consts::G__FILE__.into())),
    );
    let line = E(
        (),
        pos.clone(),
        E_::mk_id(Id(pos.clone(), pseudo_consts::G__LINE__.into())),
    );
    let renamed_id = class::ClassType::from_ast_name_and_mangle(e.alloc, &id.1);
    let cid = ClassId(
        (),
        pos.clone(),
        ClassId_::CI(Id(id.0.clone(), renamed_id.to_raw_string().into())),
    );

    emit_symbol_refs::add_class(e, renamed_id);

    Ok(E(
        (),
        pos.clone(),
        E_::New(Box::new((
            cid,
            vec![],
            vec![attribute_map, children_vec, filename, line],
            None,
            (),
        ))),
    ))
}
