---
pageid: expr.allof
title: allof
layout: docs
section: Expression Terms
permalink: docs/expr/allof.html
redirect_from: docs/expr/allof/
---

The `allof` expression term evaluates as true if all of the grouped expressions
also evaluated as true.  For example, this expression matches only files whose
name ends with `.txt` and that are not empty files:

    ["allof", ["match", "*.txt"], ["not", "empty"]]

Each array element after the term name is evaluated as an expression of its own:

    ["allof", expr1, expr2, ... exprN]

Evaluation of the subexpressions stops at the first one that returns false.

