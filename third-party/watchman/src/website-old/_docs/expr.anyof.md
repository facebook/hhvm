---
pageid: expr.anyof
title: anyof
layout: docs
section: Expression Terms
permalink: docs/expr/anyof.html
redirect_from: docs/expr/anyof/
---

The `anyof` expression term evaluates as true if any of the grouped expressions
also evaluated as true.  The following expression matches files whose name ends
with either `.txt` or `.md`:

    ["anyof", ["match", "*.txt"], ["match", "*.md"]]

Each array element after the term name is evaluated as an expression of its own:

    ["anyof", expr1, expr2, ... exprN]

Evaluation of the subexpressions stops at the first one that returns true.
