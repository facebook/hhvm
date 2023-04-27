#ifndef AGGREGATE_CHECK_INCLUDED
#define AGGREGATE_CHECK_INCLUDED

/* Copyright (c) 2014, 2020, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

/**
   @file
   Checks for some semantic constraints on queries using GROUP
   BY, or aggregate functions, or DISTINCT. Enforced if
   sql_mode contains 'only_full_group_by'.
*/

/**
  @defgroup AGGREGATE_CHECKS Aggregate checks of ONLY_FULL_GROUP_BY

Checks for some semantic constraints on queries using GROUP BY, or aggregate
functions, or DISTINCT (ONLY_FULL_GROUP_BY).

We call "aggregation" the operation of taking a group of rows and replacing
it with a single row. There are three types of aggregation: DISTINCT,
implicit grouping, explicit grouping.

This text describes MySQL's checks (why certain queries are rejected) and the
rationale behind them.

References:
- WL#2489 "better only_full_group_by".
- if you have access to the SQL standard, we recommend the following parts of
"SQL2011 Foundation": query expression Syntax rule 28; column reference Syntax
rule 7 and Conformance rule 2; 4.19 functional dependencies.

@section DISTINCT

DISTINCT: all identical rows in the result of SELECT are "aggregated" to
one single row - duplicates are eliminated.
If the result of the SELECT without DISTINCT is
@verbatim
1 2
3 4
1 2
@endverbatim
then the result with DISTINCT is
@verbatim
1 2
3 4
@endverbatim
Here is a problematic query. Assume we have a table T which contains three
columns C1,C2,C3 and has the following rows:
@verbatim
C1 C2 C3
1  2  A
3  4  B
1  2  C
@endverbatim
and we do "SELECT DISTINCT C1, C2 FROM T ORDER BY C3".
Because we want the result to be ordered, we have to first eliminate
duplicates then order the result.
When we eliminate duplicates, should we keep the first or the third row? This
arbitrary choice will influence the retained value of C3, which will influence
ordering. In the end, we have arbitrary ordering, which is problematic.
To prevent this, if, in a query block 'sl', an ORDER BY expression
is not the same expression as one in the SELECT list of 'sl'
and contains a column which:
- is of a table whose qualifying query block is 'sl'
- and is not in the SELECT list of 'sl'

then 'sl' should not have DISTINCT.
This rule makes the query above invalid.
Class Check_distinct implements this rule.

Class Check_distinct implements a second rule, specific of set functions in
ORDER BY (a non-standard feature).
Consider these queries labelled (1), (2), (3), with DISTINCT and a set
function in ORDER BY:
@verbatim
SELECT DISTINCT MIN(C2) FROM T GROUP BY C1 ORDER BY MIN(C3);
SELECT DISTINCT MIN(C2) FROM T GROUP BY C1 ORDER BY MIN(C2);
SELECT DISTINCT C1, MIN(C2) FROM T GROUP BY C1 ORDER BY MIN(C3);
@endverbatim
(1) is a random-order query, (2) and (3) are not.
MySQL has traditionally been permissive, we want to allow (2) and (3).

So the second rule is that Check_distinct rejects a query if it has DISTINCT
and a set function in ORDER BY which is not the same expression as one in the
SELECT list. This rejects (1) and allows (2).
It would reject (3); luckily, before Check_distinct checks, DISTINCT is
removed (it is redundant with GROUP BY) and thus the query is not processed by
Check_distinct; the second rule is thus by-passed and (3) is correctly
accepted.

The implementation of Check_distinct works like this: if the query has
DISTINCT, examine each element of the ORDER BY list: if the element is not the
same expression as one in the SELECT list, then examine parts of the element
(using Item::walk()), to spot offending set functions or columns.

@section IMPLICIT_GROUPING Implicit grouping

A query with set functions without GROUP BY can be seen as having GROUP BY()
i.e. the set of grouping expressions is empty, all rows are part of a
single group and are replaced with a single row. So it is just a sub-case of
the next section.

@section EXPLICIT_GROUPING Explicit grouping (GROUP BY)

MySQL is more permissive than the standard, it allows to group by any
expression; it does not require that every element of the GROUP BY clause
should be a column of one table of the FROM clause.

Here is a problematic query, using a table T with columns C1, C2, C3:
@verbatim
C1 C2 C3
1  2  A
3  4  B
1  2  C

SELECT C1, C3 FROM T GROUP BY C1;
@endverbatim
we first form groups, in each group the value of C1 must be the same for all
rows. Consider the group made of the first and third row. We have to produce
one row out of it, and this row must have a value for C3 because C3 is in the
SELECT list. Among those two rows, which value of C3 should we choose? This is
arbitrary, and will give a random result.

To prevent this, in a query with GROUP BY or aggregates (known as "a grouped
query"), any column referenced by an expression in the SELECT list or HAVING
condition and belonging to one table of the FROM clause, should be one of the
group columns (enforced by only_full_group_by in MySQL 5.6 and older) or, if
functional dependencies are supported (as in MySQL 5.7), should be
functionally dependent on the group columns.
In the table T above, C3 is obviously not functionally dependent on {C1,C2}:
the values of C1 and C2 for a row do not uniquely determine the value of
C3. In other words, if two rows have the same value in C1 and the same value
in C2 they do not necessarily have the same value in C3.
So this rule correctly rejects the query above.

Note that NULL is treated as one value: two NULLs are considered equal.

In WL#2489, we have implemented the optional standard feature T301
"functional dependencies" almost entirely.

Here are the functional dependencies which we recognize.

@subsection KEYFD Key-based, in a base table

A key in this text is a unique constraint made of only non-NULLable
columns. For example, a primary key.
Considering a base table T, if two rows have the same values of all columns of
a key of T they are actually one single row, so:
{ all columns of key} -> { T.* } (notation: what is on the right of the arrow
is functionally dependent on what is on the left).

@subsection GCOLFD Generated-column-based, in a base table

Considering a base table T, a generated column is functionally dependent on
the set of columns it references (the so-called parametric
columns). Note that the SQL standard doesn't allow a parametric column
to be a generated column, but MySQL does.

@subsection INNEREQ Equality-based, in the result of an inner join

Considering two tables T1 and T2, a condition C, and the result R of an inner
join T1 JOIN T2 ON C.
Note that T1/T2 are not necessarily base tables. For example, they can also be
views, or derived tables, or the results of some joins; in the rest of text,
we use the vague term "table" for those various objects.

Note that C may only refer to columns of T1 or T2 (outer references
are forbidden by MySQL in join conditions).

Assuming that C is a conjunction (i.e. is made of one or more conditions,
"conjuncts", chained together with AND):
- If one conjunct is of the form T1.A=constant, then {} -> {A} holds in R (the
value of A is "the constant" in all rows of R).
- If one conjunct is of the form T1.A=T2.B, then {T1.A} -> {T2.B} (and vice
versa) holds in R (the value of T2.B is that of T1.A in all rows of R).

@subsection OUTEREQ Equality-based, in the result of an outer join

Considering the result R of TS LEFT JOIN TW ON C.
Assuming that C is a conjunction. TS is said to be the strong table, TW is
said to be the weak table (the one which might be NULL-complemented in the
result of this join).
To make this really clear, note that, if we have
"t1 left join (t2 left join t3 on C) on D":
- in the t2-t3 join, t2 is strong and t3 is weak.
- in the t1-(t2-t3) join, t1 is strong, t2 is weak, t3 is weak.

If C is deterministic and one conjunct is of the form TW.A=constant or
TW.A=TS.B, then DJS -> {TW.A} holds in R, where DJS is the set of all columns
of TS referenced by C.
Proof: consider in R two rows r1 and r2 which have the same values of DJS
columns. Consider r1. There are two possibilities when r1 was built from a row
of TS:
- no row in TW matched the row of TS (for no row of TW has C been true): so,
r1 is NULL-complemented for the columns of TW. Given that r2 has the same
values of DJS columns as r1, and given that C is deterministic, it is sure
that no row in TW matched when r2 was built. So r2 is also NULL-complemented
for the columns of TW. So r1 and r2 have the same value of TW.A (NULL).
- At least one row in TW matched: so, r1 contains real values from TW (not
NULL-complemented), matching C, and thus TW.A in r1 is equal to the constant
or to TS.B. Following the same reasoning as above, it is sure that it is also
the case for r2.
- In conclusion, we can see that r1 and r2 always have the same value of
TW.A.

If one conjunct is of the form TW.A=TW.B then {TW.A} -> {TW.B} holds in R
Proof: consider r1 and r2 two rows in R having the same value of TW.A. Two
cases are possible:
- this value is NULL. Then both rows are NULL-complemented (if not, the
value of TW.A in TW would be NULL, which cannot match in an equality, so C is
not true, which is absurd). Thus, in r1 and r2 TW.B is NULL.
- This value is not NULL. Then both rows are not NULL-complemented, C
matched for both, so TW.B in r1/r2 is equal to TW.A in r1/r2.
- In conclusion, r1 and r2 have the same value of TW.B.

If any conjunct references column T1.A and is not true if T1.A is NULL
(e.g. conjunct is T1.A > 3), and T1 is part of TW, we can replace T1 with
 (SELECT * FROM T1 WHERE T1.A IS NOT NULL) AS T1.
We do not do such query rewriting, but a consequence is that we can and do
treat T1.A as "known not nullable in T1".
Proof: if we do this replacement, it means we remove from T1 some rows where
T1.A is NULL. For the sake of the proof, let's broaden this to "we remove from
and/or add to T1 rows where T1.A is NULL". By this operation,
- the result of T2 JOIN T1 ON C is affected in the same manner (i.e. it's like
if we remove from and/or add rows to the join's result, rows where T1.A is
NULL).
- the same is true for the result of T1 LEFT JOIN T2 ON C,
- regarding the result of T2 LEFT JOIN T1 ON C, consider a row r2 from T2:
   - if r2 had a match with a row of T1 where T1.A is not NULL, this match is
   preserved after replacement of T1
   - due to removed rows, matches with rows of T1 where T1.A is NULL may be
   lost, and so NULL-complemented rows may appear
   - due to added rows, matches with rows of T1 where T1.A is NULL may appear
   - so in the result of the left join, it's like if we remove from
   and/or add rows which all have T1.A is NULL.
So, the net effect in all situations, on the join nest simply containing T1,
is that "we remove from and/or add to the nest's result rows where T1.A is
NULL".
By induction we can go up the nest tree, for every nest it's like if we remove
from and/or add to the join nest's result rows where T1.A is NULL.
Going up, finally we come to the nest TS LEFT JOIN TW ON C where our
NULL-rejecting conjunct is.
If T1 belonged to TS, we could not say anything. But we have assumed T1
belongs to TW: then the C condition eliminates added rows, and would have
eliminated removed rows anyway. Thus the result of TS LEFT JOIN TW ON C is
unchanged by our operation on T1. We can thus safely treat T1.A as not
nullable _in_T1_. Thus, if T1.A is part of a unique constraint, we may treat
this constraint as a primary key of T1 (@see WHEREEQ for an example).

@subsection WHEREEQ Equality-based, in the result of a WHERE clause

Same rule as the result of an inner join.
Additionally, the rule is extended to T1.A=outer_reference, because an outer
reference is a constant during one execution of this query.

Moreoever, if any conjunct references column T1.A and is not true if T1.A is
NULL (e.g. conjunct is T1.A > 3), we can treat T1.A as not nullable in T1.
Proof: same as in previous section OUTEREQ.
We can then derive FD information from a unique index on a nullable column;
consider:
 create table t1(a int null, b int null, unique(a));
While
 select a,b from t1 group by a;
is invalid (because two null values of 'a' go in same group),
 select a,b from t1 where 'a' is not null group by a;
is valid: 'a' can be treated as non-nullable in t1, so the unique index is
like "unique not null", so 'a' determines 'b'.

Below we examine how functional dependencies in a table propagate to its
containing join nest.

@subsection PROPAGOUTER Considering the result R of TS LEFT JOIN TW ON C.

All functional dependencies in TS are also functional dependencies in R.
Proof: trivial.
The same is not necessarily true for TW.
Let's define a "NULL-friendly functional dependency" (NFFD) as a functional
dependency between two sets A and B, which has two properties:
- A is not empty
- if, in a row, all values of A are NULL, then all values of B are NULL.

All NFFDs in TW are also NFFDs in R.
Proof: consider an NFFD A -> B in TW, and r1 and r2 two rows in R having the
same values of A columns. Two cases are possible:
- In r1 and r2, at least one value of A is not NULL. Then r1 is not
NULL-complemented. Its values for A and B come from TW. By application of the
functional dependency in TW, because values in A are equal in TW, values in B
are equal in TW and thus in r1/r2.
- In r1 and r2, all values of A are NULL. Two cases are possible:
i) r1 is not NULL-complemented.  Its values for A and B come from TW. In the
row of TW values of A are all NULL. Because the functional dependency in
NULL-friendly, all values of B are NULL in the row of TW and thus in r1.
ii) r1 is NULL-complemented. Then all values of B in r1 are NULL.
iii) In conclusion, all values of B in r1 are NULL. The same reasoning applies
to r2. So, values of B are identical (NULL) in r1 and r2.
- In conclusion, values of B are identical in r1/r2, we have proved that
this is a functional dependency in R, and a NULL-friendly one.

The concept of an NFFD is Guilhem's invention. It was felt it was necessary, to
have easy propagation of FDs from TW to R. It was preferred to the
alternative, simpler rule which says that a functional dependency A-> B in TW
is also a functional dependency in R if A contains a non-nullable
column. There are two points to note:
- the functional dependency of the simpler rule is an NFFD, so our rule is not
more restrictive than the simpler one
- this simpler rule prevents free propagation of functional dependencies
through join nests, which complicates implementation and leads to rejecting
queries which could be accepted. An example follows:
@verbatim
SELECT T3.A
FROM T1 LEFT JOIN (T2 LEFT JOIN T3 ON  TRUE) ON  TRUE
GROUP BY T3.PK;
@endverbatim
This is what the simpler rule says for this query:
* In T3, T3.PK->T3.A holds.
* Let R1 be the result of "(T2 LEFT JOIN T3 ON TRUE)", in R1 T3.PK->T3.A
holds, by application of: there is a functional dependency in the
weak side T3, and T3.PK is not nullable in T3.
* Let R2 be the result of "T1 LEFT JOIN (T2 LEFT JOIN T3 ON TRUE) ON TRUE",
in R2 T3.PK->T3.A doesn't hold anymore, because: it's a dependency in the
weak side (weak side is R1 here), and T3.PK is nullable _when
seen as a column of R1_ (in R1 T3.PK can be NULL, if the row of T3
is actually a NULL-complemented one).

@subsection PROPAGINNER Considering the result R of T1 JOIN T2 ON C.

All [NULL-friendly] functional dependencies in T1 are also [NULL-friendly]
functional dependencies in R. the same is true for T2.
Proof: trivial.

@subsection PROPAGSUMMARY Summary rules for propagating FDs

All NULL-friendly functional dependencies propagate freely through join nests
all the way up to the result of the WHERE clause.
The same is true for ordinary functional dependencies except if there are weak
tables along the propagation path between the table where the dependency holds
and the result of the WHERE clause; in other words, except if the table where
the dependency holds belongs to some embedding join nest which is on the weak
side of an outer join.

@subsection NFFDS Which functional dependencies are NULL-friendly

A key-based functional dependency A -> B in the base table is NULL-friendly,
because, by definition, there can never be a NULL value in any column of A.

A functional dependency A -> B in a base table, between parametric columns A
and a generated column B, is not NULL-friendly; for more details, see
@ref FDVIEW .

A functional dependency A->B in the result of T1 JOIN T2 ON C, if based on
equality of two columns, is NULL-friendly. Indeed, A is not empty and if there
was some NULL in A, it would not match the equality in C and thus it would not
exist in the result, absurd. If the equality is rather column=constant, A is
empty, the dependency is not NULL-friendly. However, in our implementation,
function @c simplify_joins() moves inner-join conditions to the embedding
outer-join nest's join condition, or to the WHERE clause. Because our analysis
of functional dependencies happens after simplify_joins(), when we analyze T1
JOIN T2 it is guaranteed to have no condition, and this paragraph is
irrelevant.

A functional dependency in the result of TS LEFT JOIN TW ON C, if based on
equality of two columns, is NULL-friendly.
Proof: let's consider, in turn, the two types of equality-based functional
dependencies which exist in this result R. Let r1 be a row of R.
- If C is deterministic and one conjunct is of the form TW.A=constant or
TW.A=TS.B, then DJS -> {TW.A} holds in R, where DJS is the set of all columns
of TS referenced by C. For NULL-friendliness, we need DJS to not be
empty. Thus, we exclude the form TW.A=constant and consider only
TW.A=TS.B. We suppose that in r1 DJS contains all NULLs. Conjunct is TW.A=TS.B
then this equality is not true, so r1 is NULL-complemented: TW.A is NULL in
r1.
- If one conjunct is of the form TW.A=TW.B then {TW.A} -> {TW.B} holds in
R. If in r1 TW.A is NULL, again the equality in C is not true, and TW.B is
NULL in R1.
- In conclusion, this is NULL-friendly.

A functional dependency in the result of a WHERE clause, if based on equality
of two columns, is NULL-friendly. If based on T1.A=constant, it is not, as it
has an empty set of source columns.

Summary: all functional dependencies which we have seen so far are
NULL-friendly, except those inferred from TW.A=constant in an outer join
condition or in a WHERE clause, and those about generated columns.

Thus, in the query with T1-T2-T3 previously seen, T3.PK->T3.A is
NULL-friendly and propagates, query is accepted.

In our implementation, we take special care of TW.A=constant in an outer join
condition: we infer a functional dependency DJS->TW.A from such equality only
if one of these requirements are met:
- the join nest "TS LEFT JOIN TW ON TW.A=constant [AND...]" is not on the
weak side of any embedding join nest - in that case, propagation will not meet
any weak tables so we do not need the dependency to be NULL-friendly, it will
propagate anyway.
- DJS contains at least one column from a strong-side table which, if NULL,
makes the join condition not evaluate to TRUE - in that case, DJS->TW.A is
NULL-friendly.

Either of these two conditions is satisfied in most practical cases. For
example, it's very common to have an equality between a strong-side column and
a weak-side column as a conjunct in the outer join condition (like, "ON
strong.pk = weak.foreign_key AND ..." or "ON strong.foreign_key = weak.pk AND
..."); this satisfies the second condition. It's also common to have outer
joins only left-deep ("SELECT ... T1 LEFT JOIN T2 ON ... LEFT JOIN T3 ON ..."
is left-deep); this satisfies the first condition.
Note that the dependency found from TW.A=TS.B in an outer join condition
always satisfies the second condition.

T1.A=constant in a WHERE clause is exterior to any join nest so does not need
to propagate, so does not need to be NULL-friendly.

@subsection FDVIEW Functional dependencies in a view or a derived table

In the rest of this text, we will use the term "view" for "view or derived
table". A view can be merged or materialized, in MySQL.
Consider a view V defined by a query expression.
If the query expression contains UNION or ROLLUP (which is theoretically based
on UNION) there are no functional dependencies in this view.
So let's assume that the query expression is a query specification (let's note
it VS):
@verbatim
CREATE VIEW V AS SELECT [DISTINCT] VE1 AS C1, VE2 AS C2, ... FROM ... WHERE ...
[GROUP BY ...] [HAVING ...] [ORDER BY ...]
@endverbatim

If {VE1, VE2, VE3} are columns of tables of the FROM clause, and {VE1,
VE2} -> {VE3} has been deduced from rules in the previous sections [and is
NULL-friendly], then {C1, C2} -> { C3 } holds in the view [and is
NULL-friendly].

If {VE1, VE2} are columns of tables of the FROM clause, and VE3 is a
deterministic expression depending only on VE1 and VE2, then {C1, C2} ->
{ C3 } in the view.
It is not always NULL-friendly, for example: VE3 could be COALESCE(VE1,VE2,3):
if VE1 (C1) and VE2 (C2) are NULL, VE3 (C3) is 3: not NULL. Another example:
VE3 could be a literal; {}->{C3}, the left set is empty.
The same examples apply to a generated column in a base table - it is like a
merged view's expression. For example, in a base table T which has a generated
column C3 AS COALESCE(C1,C2,3): {C1, C2} -> { C3 } holds in T but is not
NULL-friendly.

If VS is a grouped query (which, in MySQL, implies that the view is
materialized), then in the result of the grouping there is a functional
dependency from the set of all group expressions to the set of all selected
expressions (otherwise, this query expression would not have passed its own
only_full_group_by validation - in the implementation we validate each query
expression separately). Thus, if all group expressions of VS are in the select
list of VS, for example they are VE1 and VE2, then {C1, C2} -> {V.*}.
It is not NULL-friendly, for example: VE3 is COUNT(1): if the result of the
WHERE clause contains a row with group expressions VE1 and VE2 equal to NULL,
VE3 is not NULL.

It's possible to infer functional dependencies from equality conditions in
HAVING, but we have not implemented it yet.

Because some functional dependencies above are not NULL-friendly, they
exist in the view, but may not freely propagate in the result of join nests
containing the view. This includes examples just given in paragraphs above,
and the case of T1.A=constant in the WHERE clause of VS.

Thus, when we know of a functional dependency A -> B in the query expression
of a view, we deduce from it a functional dependency in the view only if:
- this view is not on the weak side of any embedding join nest (so
NULL-friendliness is not required for propagation).
- or A contains at least one non-nullable expression, which makes A -> B
NULL-friendly.

The above is fine for materialized views. For merged views, we cannot study the
query expression of the view, it has been merged (and scattered), so we use a
different rule:
- a merged view is similar to a join nest inserted in the parent query, so
for dependencies based on keys or join conditions, we simply follow
propagation rules of the non-view sections.
- For expression-based dependencies (VE3 depends on VE1 and VE2, VE3
belonging to the view SELECT list), which may not be NULL-friendly, we require
 - the same non-weak-side criterion as above
 - or that the left set of the dependency be non-empty and that if VE1 and
VE2 are NULL then VE3 must be NULL, which makes the dependency NULL-friendly.
- The same solution is used for generated columns in a base table.

  @{

*/

#include <stddef.h>
#include <sys/types.h>

#include "my_alloc.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_table_map.h"
#include "sql/item.h"
#include "sql/item_cmpfunc.h"    // Item_func_any_value
#include "sql/item_sum.h"        // Item_sum
#include "sql/mem_root_array.h"  // Mem_root_array
#include "sql/sql_lex.h"

class Opt_trace_context;
class Opt_trace_object;
class THD;
struct TABLE_LIST;
template <class T>
class List;

/**
   Checks for queries which have DISTINCT.
*/
class Distinct_check : public Item_tree_walker {
 public:
  Distinct_check(SELECT_LEX *select_arg)
      : select(select_arg), failed_ident(nullptr) {}

  bool check_query(THD *thd);

 private:
  /// Query block which we are validating
  SELECT_LEX *const select;
  /// Identifier which triggered an error
  Item_ident *failed_ident;

  /**
     Just because we need to go through Item::walk() to reach all items to
     validate, some work must be delegated to "Item processors" (members of
     Item); this work conceptually belongs to Distinct_check, and needs
     privileged access to it.
  */
  friend bool Item_sum::aggregate_check_distinct(uchar *arg);
  friend bool Item_ident::aggregate_check_distinct(uchar *arg);
  friend bool Item_func_any_value::aggregate_check_distinct(uchar *arg);

  FORBID_COPY_CTOR_AND_ASSIGN_OP(Distinct_check);
};

/**
   Checks for queries which have GROUP BY or aggregate functions.
*/
class Group_check : public Item_tree_walker {
 public:
  Group_check(SELECT_LEX *select_arg, MEM_ROOT *root)
      : select(select_arg),
        search_in_underlying(false),
        non_null_in_source(false),
        table(nullptr),
        group_in_fd(~0ULL),
        m_root(root),
        fd(root),
        whole_tables_fd(0),
        recheck_nullable_keys(0),
        mat_tables(root),
        failed_ident(nullptr) {}

  ~Group_check() {
    for (uint j = 0; j < mat_tables.size(); ++j) destroy(mat_tables.at(j));
  }

  bool check_query(THD *thd);
  void to_opt_trace(THD *thd);

 private:
  /// Query block which we are validating
  SELECT_LEX *const select;

  /**
     "Underlying" == expressions which are underlying in an identifier.
     The identifier can be a column of a view or derived table, both merged or
     materialized, or a generated column: all of those have an underlying
     expression.
     "Materialized table (mat table)" == view or derived table, materialized.
     If this is true, is_in_fd() will look for FDs in underlying expressions
     of columns.
  */
  bool search_in_underlying;

  /**
     This member is readable only if this is a child Group_check. Is true if
     one expression of the SELECT list of this->select is non-nullable.
  */
  bool non_null_in_source;

  /**
     The Group_check employed to validate one query block, the one on which
     check_query() runs, is named the "master".
     If the query block references a materialized table, the master may create
     a child Group_check, whose job is to discover FDs in the query expression
     of the mat table (with the ultimate goal of deducing from them some FDs
     in the mat table and thus in the parent Group_check). A child may have
     children itself. If this Group_check is a child, 'table' points to the
     materialized table, otherwise it is NULL.
  */
  TABLE_LIST *const table;

  /**
     Bit N is set if the N-th expression of GROUP BY is functionally dependent
     on source columns.
     It serves to find FDs in the query expression of a materialized table
     having GROUP BY.
     For a non-child Group-check, all bits are turned on from the start.
     Currently limited to 64 bits => max 64 GROUP expressions; should probably
     be MY_BITMAP.
  */
  ulonglong group_in_fd;

  /// Memory for allocations (like of 'fd')
  MEM_ROOT *const m_root;

  /**
     Columns which are local to 'select' and functionally dependent on an
     initial set of "source" columns defined like this:
     - if !is_child(), the GROUP BY columns
     - if is_child(), columns of the result of the query expression under
     'table' which are themselves part of 'fd' of the parent Group_check.
  */
  Mem_root_array<Item_ident *> fd;
  /// Map of tables for which all columns can be considered part of 'fd'.
  table_map whole_tables_fd;
  /// Map of tables for which we discovered known-not-nullable columns.
  table_map recheck_nullable_keys;
  /// Children Group_checks of 'this'
  Mem_root_array<Group_check *> mat_tables;
  /// Identifier which triggered an error
  Item_ident *failed_ident;

  bool is_fd_on_source(Item *item);
  bool is_child() const { return table != nullptr; }

  /// Private ctor, for a Group_check to build a child Group_check
  Group_check(SELECT_LEX *select_arg, MEM_ROOT *root, TABLE_LIST *table_arg)
      : select(select_arg),
        search_in_underlying(false),
        non_null_in_source(false),
        table(table_arg),
        group_in_fd(0ULL),
        m_root(root),
        fd(root),
        whole_tables_fd(0),
        recheck_nullable_keys(0),
        mat_tables(root) {
    DBUG_ASSERT(table);
  }
  bool check_expression(THD *thd, Item *expr, bool in_select_list);
  /// Shortcut for common use of Item::local_column()
  bool local_column(Item *item) const {
    return item->local_column(select).is_true();
  }
  void add_to_fd(Item *item, bool local_column, bool add_to_mat_table = true);
  void add_to_fd(table_map m) {
    whole_tables_fd |= m;
    find_group_in_fd(nullptr);
  }
  void add_to_source_of_mat_table(Item_field *item_field, TABLE_LIST *tl);
  bool is_in_fd(Item *item);
  bool is_in_fd_of_underlying(Item_ident *item);
  Item *get_fd_equal(Item *item);
  void analyze_conjunct(Item *cond, Item *conjunct, table_map weak_tables,
                        bool weak_side_upwards);
  void analyze_scalar_eq(Item *cond, Item *left_item, Item *right_item,
                         table_map weak_tables, bool weak_side_upwards);
  void find_fd_in_cond(Item *cond, table_map weak_tables,
                       bool weak_side_upwards);
  void find_fd_in_joined_table(mem_root_deque<TABLE_LIST *> *join_list);
  void to_opt_trace2(Opt_trace_context *ctx, Opt_trace_object *parent);
  void find_group_in_fd(Item *item);
  Item *select_expression(uint idx);

  /// Enum for argument of do_ident_check()
  enum enum_ident_check { CHECK_GROUP, CHECK_STRONG_SIDE_COLUMN, CHECK_COLUMN };
  bool do_ident_check(Item_ident *i, table_map tm, enum enum_ident_check type);

  /**
     Just because we need to go through Item::walk() to reach all items to
     validate, some work must be delegated to "Item processors" (members of
     Item); this work conceptually belongs to Group_check, and needs
     privileged access to it.
  */
  friend bool Item_ident::aggregate_check_group(uchar *arg);
  friend bool Item_sum::aggregate_check_group(uchar *arg);
  friend bool Item_func_any_value::aggregate_check_group(uchar *arg);
  friend bool Item_ident::is_strong_side_column_not_in_fd(uchar *arg);
  friend bool Item_ident::is_column_not_in_fd(uchar *arg);
  friend bool Item_func_grouping::aggregate_check_group(uchar *arg);

  FORBID_COPY_CTOR_AND_ASSIGN_OP(Group_check);
};

#endif
/// @} (end of group AGGREGATE_CHECKS ONLY_FULL_GROUP_BY)
