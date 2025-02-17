// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

// Tests in their own file because we want to use hack_macros to build them, but
// hack_macros depends on the lowerer, which contains
// desugar_expression_tree.rs, so if we test there we have a cyclic dependency

#[cfg(test)]
mod tests {
    use std::collections::HashSet;

    use hack_macros::hack_stmts;
    use lowerer::desugar_expression_tree::LiveVars;

    pub fn live_vars_from_vecs(used: Vec<&str>, assigned: Vec<&str>) -> LiveVars {
        LiveVars {
            used: HashSet::from_iter(used.into_iter().map(|s| (0, s.to_string()))),
            assigned: HashSet::from_iter(assigned.into_iter().map(|s| (0, s.to_string()))),
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
}
