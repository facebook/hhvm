use decl_provider::DeclProvider;
use hhbc_by_ref_emit_symbol_refs as emit_symbol_refs;
use hhbc_by_ref_env::emitter::Emitter;
use hhbc_by_ref_hhbc_id::{class, Id};
use hhbc_by_ref_instruction_sequence::Result;
use naming_special_names_rust::pseudo_consts;
use oxidized::{
    aast_visitor::{visit_mut, AstParams, NodeMut, VisitorMut},
    ast as tast, ast_defs,
    pos::Pos,
};

struct RewriteXmlVisitor<'emitter, 'arena, 'decl, D> {
    phantom: std::marker::PhantomData<(&'emitter &'arena (), &'emitter &'decl (), D)>,
}

struct Ctx<'emitter, 'arena, 'decl, D: DeclProvider<'decl>> {
    alloc: &'arena bumpalo::Bump,
    emitter: &'emitter mut Emitter<'arena, 'decl, D>,
}

impl<'ast, 'arena, 'emitter, 'decl, D: DeclProvider<'decl> + 'emitter> VisitorMut<'ast>
    for RewriteXmlVisitor<'emitter, 'arena, 'decl, D>
{
    type P = AstParams<Ctx<'emitter, 'arena, 'decl, D>, hhbc_by_ref_instruction_sequence::Error>;

    fn object(&mut self) -> &mut dyn VisitorMut<'ast, P = Self::P> {
        self
    }

    fn visit_expr(
        &mut self,
        c: &mut Ctx<'emitter, 'arena, 'decl, D>,
        e: &'ast mut tast::Expr,
    ) -> Result<()> {
        let tast::Expr(pos, expr) = e;
        let alloc = &c.alloc;
        let emitter = &mut c.emitter;
        if let tast::Expr_::Xml(cs) = expr {
            *e = rewrite_xml_(alloc, emitter, pos, cs.as_ref().clone())?;
        }
        e.recurse(c, self.object())?;
        Ok(())
    }
}

pub fn rewrite_xml<'p, 'arena, 'emitter, 'decl, D: DeclProvider<'decl>>(
    alloc: &'arena bumpalo::Bump,
    emitter: &'emitter mut Emitter<'arena, 'decl, D>,
    prog: &'p mut tast::Program,
) -> Result<()> {
    let mut xml_visitor = RewriteXmlVisitor {
        phantom: std::marker::PhantomData,
    };
    let mut c: Ctx<'emitter, 'arena, 'decl, D> = Ctx { alloc, emitter };

    visit_mut(&mut xml_visitor, &mut c, prog)
}

fn rewrite_xml_<'arena, 'decl, D: DeclProvider<'decl>>(
    alloc: &'arena bumpalo::Bump,
    e: &mut Emitter<'arena, 'decl, D>,
    pos: &Pos,
    (id, attributes, children): (tast::Sid, Vec<tast::XhpAttribute>, Vec<tast::Expr>),
) -> Result<tast::Expr> {
    use ast_defs::{Id, ShapeFieldName as SF};
    use tast::{ClassId, ClassId_, Expr as E, Expr_ as E_, XhpAttribute};

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
                                expr.0.clone(),
                                format!("...${}", spread_id.to_string()).into(),
                            )),
                            expr,
                        ));
                        spread_id += 1;
                    }
                }
                (spread_id, attrs)
            });
    let attribute_map = E(pos.clone(), E_::mk_shape(attributes));
    let children_vec = E(pos.clone(), E_::mk_varray(None, children));
    let filename = E(
        pos.clone(),
        E_::mk_id(Id(pos.clone(), pseudo_consts::G__FILE__.into())),
    );
    let line = E(
        pos.clone(),
        E_::mk_id(Id(pos.clone(), pseudo_consts::G__LINE__.into())),
    );
    let renamed_id = class::Type::from_ast_name_and_mangle(alloc, &id.1);
    let cid = ClassId(
        pos.clone(),
        ClassId_::CI(Id(id.0.clone(), renamed_id.to_raw_string().into())),
    );

    emit_symbol_refs::add_class(alloc, e, renamed_id);

    Ok(E(
        pos.clone(),
        E_::New(Box::new((
            cid,
            vec![],
            vec![attribute_map, children_vec, filename, line],
            None,
            pos.clone(),
        ))),
    ))
}
