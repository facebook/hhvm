// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

// Tests in their own file because we want to use hack_macros to build them, but
// hack_macros depends on the lowerer, which contains
// desugar_expression_tree.rs, so if we test there we have a cyclic dependency

#[cfg(test)]
mod tests {
    use std::collections::HashMap;

    use hack_macros::hack_expr;
    use hack_macros::hack_stmts;
    use lowerer::desugar_expression_tree::LiveVars;

    fn strings_to_map(v: Vec<&str>) -> HashMap<aast::LocalId, Pos> {
        HashMap::from_iter(v.iter().map(|s| ((0, s.to_string()), Pos::NONE)))
    }

    fn strings_to_lids(v: Vec<&str>) -> Vec<aast::Lid> {
        v.iter()
            .map(|s| aast::Lid(Pos::NONE, (0, s.to_string())))
            .collect()
    }

    fn live_vars_from_vecs(used: Vec<&str>, assigned: Vec<&str>) -> LiveVars {
        LiveVars {
            used: strings_to_map(used),
            assigned: strings_to_map(assigned),
        }
    }

    #[test]
    fn expr() {
        let lvs = LiveVars::new_from_statement(&hack_stmts!("$a + $b;"));
        let res = live_vars_from_vecs(vec!["$a", "$b"], vec![]);
        assert_eq!(lvs, res);
    }

    #[test]
    fn seq() {
        let lvs = LiveVars::new_from_statement(&hack_stmts!("$a; $x = $y; $y = $z; $a = $x;"));
        let res = live_vars_from_vecs(vec!["$y", "$z", "$a"], vec!["$x", "$y", "$a"]);
        assert_eq!(lvs, res);
    }

    #[test]
    fn reassign() {
        let lvs = LiveVars::new_from_statement(&hack_stmts!("$a = $a + 1;"));
        let res = live_vars_from_vecs(vec!["$a"], vec!["$a"]);
        assert_eq!(lvs, res);
    }

    #[test]
    fn iftest() {
        let lvs = LiveVars::new_from_statement(&hack_stmts!(
            "$a2 = 1; $x = 1; if ($a + $a2) { $a = $z + $b; $c = $x; } else { $f= 1; $f; $c = $y; $d = $z;}; $b = $c + $d; $x;"
        ));
        let res = live_vars_from_vecs(
            vec!["$a", "$z", "$y", "$d", "$b"],
            vec!["$a2", "$b", "$c", "$x"],
        );
        assert_eq!(lvs, res);
    }

    #[test]
    fn whiletest() {
        let lvs = LiveVars::new_from_statement(&hack_stmts!(
            "$c = 1; $d = 2; while ($x + $d) { $a = $b; $x = $c;}; $a;"
        ));
        let res = live_vars_from_vecs(vec!["$x", "$a", "$b"], vec!["$c", "$d"]);
        assert_eq!(lvs, res);
    }

    #[test]
    fn fortest_simple() {
        let lvs = LiveVars::new_from_statement(&hack_stmts!(
            "$c = 1; $d = 2; for (;$x + $d;) { $a = $b; $x = $c;}; $a;"
        ));
        let res = live_vars_from_vecs(vec!["$x", "$a", "$b"], vec!["$c", "$d"]);
        assert_eq!(lvs, res);
    }

    #[test]
    fn foreach_init() {
        let lvs = LiveVars::new_from_statement(&hack_stmts!(
            "$a = 1; for ($b = $c, $d = $b; $b + $e;) { $d; $c = 4;}; $b;"
        ));
        let res = live_vars_from_vecs(vec!["$c", "$e"], vec!["$a", "$b", "$d"]);
        assert_eq!(lvs, res);
    }

    #[test]
    fn foreach_inc() {
        let lvs = LiveVars::new_from_statement(&hack_stmts!(
            "for ($d = $b0; $e; $b = 1, $e = 1, $b1 = 1, $b0 = $b1, $b2 = $c) { $b2; $d; $c = 4;}; $b;"
        ));
        let res = live_vars_from_vecs(vec!["$e", "$b", "$b0", "$b2"], vec!["$d"]);
        assert_eq!(lvs, res);
    }

    #[test]
    fn lambda() {
        let lvs = LiveVars::new_from_statement(&hack_stmts!(
            "$x = 1; ($a ==> {$x; $a; $e; $y = 1; $z = 1;})($y); $z;"
        ));
        let res = live_vars_from_vecs(vec!["$e", "$y", "$z"], vec!["$x"]);
        assert_eq!(lvs, res);
    }

    #[test]
    fn et() {
        let lvs = hack_expr!("ET`$x`")
            .2
            .as_expression_tree()
            .unwrap()
            .free_vars
            .clone();
        let res = None;
        assert_eq!(lvs, res);
    }

    #[test]
    fn nest_et() {
        let splices = get_splices(&hack_expr!("ET`${ET`{$y = 1; $x + $y;}`}`"));
        let fvs = splices[0]
            .spliced_expr
            .2
            .as_expression_tree()
            .unwrap()
            .free_vars
            .clone();
        let res = Some(strings_to_lids(vec!["$x"]));
        assert_eq!(fvs, res);
        assert_eq!(splices[0].macro_variables, res);
    }

    #[test]
    fn nest_et_no_vars() {
        let splices = get_splices(&hack_expr!("ET`${ET`{$y = 1; $y;}`}`"));
        let fvs = splices[0]
            .spliced_expr
            .2
            .as_expression_tree()
            .unwrap()
            .free_vars
            .clone();
        let res = Some(vec![]);
        assert_eq!(fvs, res);
        assert_eq!(splices[0].macro_variables, None);
    }

    #[test]
    fn nest_multiple_et() {
        let splices = get_splices(&hack_expr!("ET`${ET`$y` + ET`$z`}`"));
        let res = Some(strings_to_lids(vec!["$y", "$z"]));
        assert_eq!(splices[0].macro_variables, res);
    }

    #[test]
    fn double_nested_et() {
        let expr = hack_expr!("ET`${ET`${ET`$x`}`}`");
        let splices = get_splices(&expr);
        let nested_splices = get_splices(&splices[0].spliced_expr);
        let fvs = splices[0]
            .spliced_expr
            .2
            .as_expression_tree()
            .unwrap()
            .free_vars
            .clone();
        let res = Some(strings_to_lids(vec!["$x"]));
        assert_eq!(nested_splices[0].macro_variables, res);
        assert_eq!(fvs, res);
        //assert_eq!(splices[0].macro_variables, res);
    }

    use oxidized::aast;
    use oxidized::aast_visitor::*;
    use oxidized::pos::Pos;

    // Helper to find all of the splices in an expression
    fn get_splices(expr: &aast::Expr<(), ()>) -> Vec<aast::EtSplice<(), ()>> {
        let mut fs = FindSplices::default();
        let _ = fs.visit_expr(&mut (), expr);
        fs.splices
    }

    #[derive(Default)]
    struct FindSplices {
        splices: Vec<aast::EtSplice<(), ()>>,
    }

    impl<'ast> Visitor<'ast> for FindSplices {
        type Params = AstParams<(), ()>;

        fn object(&mut self) -> &mut dyn Visitor<'ast, Params = Self::Params> {
            self
        }

        fn visit_et_splice(
            &mut self,
            _: &mut (),
            splice: &aast::EtSplice<(), ()>,
        ) -> Result<(), ()> {
            self.splices.push(splice.clone());
            Ok(())
        }
    }
}
