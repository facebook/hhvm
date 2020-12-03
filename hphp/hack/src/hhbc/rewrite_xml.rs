use emit_symbol_refs_rust as emit_symbol_refs;
use env::emitter::Emitter;
use hhbc_id_rust::{class, Id};
use instruction_sequence_rust::Result;
use naming_special_names_rust::pseudo_consts;
use oxidized::{
    aast_visitor::{visit_mut, AstParams, NodeMut, VisitorMut},
    ast as tast, ast_defs,
    pos::Pos,
};

struct RewriteXmlVisitor {}

impl<'ast> VisitorMut<'ast> for RewriteXmlVisitor {
    type P = AstParams<Emitter, instruction_sequence_rust::Error>;

    fn object(&mut self) -> &mut dyn VisitorMut<'ast, P = Self::P> {
        self
    }

    fn visit_expr(&mut self, emitter: &mut Emitter, e: &'ast mut tast::Expr) -> Result<()> {
        let tast::Expr(ref pos, ref expr) = e;
        match expr {
            tast::Expr_::Xml(cs) => {
                *e = rewrite_xml_(emitter, pos, cs.as_ref())?;
                e.recurse(emitter, self.object())?
            }
            _ => e.recurse(emitter, self.object())?,
        }

        Ok(())
    }
}

pub fn rewrite_xml<'p>(emitter: &mut Emitter, prog: &'p mut tast::Program) -> Result<()> {
    let mut xml_visitor = RewriteXmlVisitor {};

    visit_mut(&mut xml_visitor, emitter, prog)
}

fn rewrite_xml_(
    e: &mut Emitter,
    pos: &Pos,
    (id, attributes, children): &(tast::Sid, Vec<tast::XhpAttribute>, Vec<tast::Expr>),
) -> Result<tast::Expr> {
    use ast_defs::{Id, ShapeFieldName as SF};
    use tast::{ClassId, ClassId_, Expr as E, Expr_ as E_, XhpAttribute};

    // TODO(hrust): avoid clone
    let (_, attributes) =
        attributes
            .iter()
            .fold((0, vec![]), |(mut spread_id, mut attrs), attr| {
                match attr {
                    XhpAttribute::XhpSimple((pos, name), v) => {
                        attrs.push((SF::SFlitStr((pos.clone(), name.clone().into())), v.clone()));
                    }
                    XhpAttribute::XhpSpread(expr) => {
                        attrs.push((
                            SF::SFlitStr((
                                expr.0.clone(),
                                format!("...${}", spread_id.to_string()).into(),
                            )),
                            expr.clone(),
                        ));
                        spread_id += 1;
                    }
                }
                (spread_id, attrs)
            });
    let attribute_map = E(pos.clone(), E_::mk_shape(attributes));
    let children_vec = E(pos.clone(), E_::mk_varray(None, children.clone()));
    let filename = E(
        pos.clone(),
        E_::mk_id(Id(pos.clone(), pseudo_consts::G__FILE__.into())),
    );
    let line = E(
        pos.clone(),
        E_::mk_id(Id(pos.clone(), pseudo_consts::G__LINE__.into())),
    );
    let renamed_id = class::Type::from_ast_name_and_mangle(&id.1);
    let cid = ClassId(
        pos.clone(),
        ClassId_::CI(Id(id.0.clone(), renamed_id.to_raw_string().into())),
    );

    emit_symbol_refs::State::add_class(e, renamed_id);

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
