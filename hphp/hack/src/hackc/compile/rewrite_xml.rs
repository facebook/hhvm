use env::emitter::Emitter;
use error::Error;
use error::Result;
use naming_special_names_rust::pseudo_consts;
use oxidized::aast_visitor::visit_mut;
use oxidized::aast_visitor::AstParams;
use oxidized::aast_visitor::NodeMut;
use oxidized::aast_visitor::VisitorMut;
use oxidized::ast;
use oxidized::ast_defs;
use oxidized::pos::Pos;

struct RewriteXmlVisitor<'emitter, 'arena, 'decl> {
    phantom: std::marker::PhantomData<(&'emitter &'arena (), &'emitter &'decl ())>,
}

struct Ctx<'emitter, 'arena, 'decl> {
    emitter: &'emitter mut Emitter<'arena, 'decl>,
}

impl<'ast, 'arena, 'emitter, 'decl> VisitorMut<'ast>
    for RewriteXmlVisitor<'emitter, 'arena, 'decl>
{
    type Params = AstParams<Ctx<'emitter, 'arena, 'decl>, Error>;

    fn object(&mut self) -> &mut dyn VisitorMut<'ast, Params = Self::Params> {
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
    use ast::ClassId;
    use ast::ClassId_;
    use ast::Expr;
    use ast::Expr_;
    use ast::XhpAttribute;
    use ast_defs::Id;
    use ast_defs::ShapeFieldName;

    let (_, attributes) =
        attributes
            .into_iter()
            .fold((0, vec![]), |(mut spread_id, mut attrs), attr| {
                match attr {
                    XhpAttribute::XhpSimple(xhp_simple) => {
                        let (pos, name) = xhp_simple.name;
                        attrs.push((
                            ShapeFieldName::SFlitStr((pos, name.into())),
                            xhp_simple.expr,
                        ));
                    }
                    XhpAttribute::XhpSpread(expr) => {
                        attrs.push((
                            ShapeFieldName::SFlitStr((
                                expr.1.clone(),
                                format!("...${}", spread_id).into(),
                            )),
                            expr,
                        ));
                        spread_id += 1;
                    }
                }
                (spread_id, attrs)
            });
    let attribute_map = Expr((), pos.clone(), Expr_::mk_shape(attributes));
    let children_vec = Expr(
        (),
        pos.clone(),
        Expr_::ValCollection(Box::new(((pos.clone(), ast::VcKind::Vec), None, children))),
    );
    let filename = Expr(
        (),
        pos.clone(),
        Expr_::mk_id(Id(pos.clone(), pseudo_consts::G__FILE__.into())),
    );
    let line = Expr(
        (),
        pos.clone(),
        Expr_::mk_id(Id(pos.clone(), pseudo_consts::G__LINE__.into())),
    );
    let renamed_id = hhbc::ClassName::from_ast_name_and_mangle(&id.1);
    let cid = ClassId(
        (),
        pos.clone(),
        ClassId_::CI(Id(id.0.clone(), renamed_id.as_str().into())),
    );

    e.add_class_ref(renamed_id);

    Ok(Expr(
        (),
        pos.clone(),
        Expr_::New(Box::new((
            cid,
            vec![],
            vec![attribute_map, children_vec, filename, line],
            None,
            (),
        ))),
    ))
}
