// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

mod node;
mod node_impl;
mod node_impl_gen;
mod visitor;

pub use node::Node;
pub use visitor::Visitor;

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn test_simple_visitor() {
        use crate::{
            pos::Pos, relative_path::RelativePath, typing_defs_core::*, typing_reason::Reason,
        };
        use Ty_::*;

        struct PrintEveryTapplyVisitor<'a>(Vec<&'a Pos<'a>>, Vec<String>);

        impl<'a> Visitor<'a> for PrintEveryTapplyVisitor<'a> {
            fn object(&mut self) -> &mut dyn Visitor<'a> {
                self
            }

            fn visit_ty(&mut self, ty: &'a Ty) {
                self.0.push(ty.get_pos().unwrap());
                if let Ty_::Tapply(&((pos, id), _)) = ty.1 {
                    self.1.push(format!("Found {} on line {}", id, pos.line()));
                    for pos in self.0.iter().rev() {
                        self.1.push(format!("  in a ty on line {}", pos.line()));
                    }
                }
                ty.recurse(self.object());
                self.0.pop();
            }
        }

        let b = &bumpalo::Bump::new();
        let pos = |line| Pos::from_line_cols_offset(b, RelativePath::empty(), line, 10..50, 100);
        macro_rules! a {
            ($e:expr) => {
                &*b.alloc($e)
            };
        }
        let ty = a!(Ty(
            a!(Reason::hint(pos(1))),
            Ttuple(a!([
                a!(Ty(
                    a!(Reason::hint(pos(2))),
                    Tapply(a!(((pos(3), "foo"), &[][..]))),
                )),
                a!(Ty(
                    a!(Reason::hint(pos(4))),
                    Tapply(a!(((pos(5), "bar"), &[][..]))),
                )),
            ])),
        ));

        let mut visitor = PrintEveryTapplyVisitor(vec![], vec![]);

        visitor.visit_ty(ty);

        assert!(visitor.0.is_empty());
        assert_eq!(
            visitor.1,
            vec![
                "Found foo on line 3",
                "  in a ty on line 2",
                "  in a ty on line 1",
                "Found bar on line 5",
                "  in a ty on line 4",
                "  in a ty on line 1",
            ]
        );
    }
}
