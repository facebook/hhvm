# Splat: merge algebra, subtyping, and inference

A design document for the `impl/` OCaml engine covering the three key additions:

1. the **merge algebra** (how spread collapses an ordered list of rows into one row),
2. the **subtyping rules** (with a long section on the type-parameter cases), and
3. **inference** (how spread variables and unification variables are solved).

File and line references point at the OCaml under `impl/lib` and the tests under
`impl/test/test_splat.ml`. The core is three files: `repr.ml` (the representation and
the algebra), `splat.ml` (subtyping and inference), and `env.ml` (variable environments
and solving). `prop.ml` is the residual-constraint tree.

Throughout, "row" is a separate syntactic kind that is never exposed to a source
program. A row becomes a type only through `shape`, and a type re-enters row position
only through `spread`. This mirrors Hack: a `shape(...)` type is a row lifted into a
type, and spreading a shape (`shape(...$s, 'k' => v)`) is the destructor.

---

## 1. Representation

All types live in `repr.ml`. There are four levels: base types, rows, field
descriptors, and requiredness.

### 1.1 Base types

`repr.ml:27-34`:

```
base =
  | Top                      -- ⊤
  | Bottom                   -- ⊥
  | Prim of Prim.t           -- Bool | Nat (stand-ins for scalar types)
  | Union of base * base     -- first-class union types
  | Shape of row             -- a row lifted into a type (Hack's shape(...))
  | Flex of Ty_var.t         -- an inference variable (unification variable)
  | Rigid of Ty_param.t      -- a bounded generic parameter (Hack's `T as ...`)
```

Hack mapping: `Prim` stands for the scalar leaves, `Union` is Hack's union type,
`Shape` is `shape(...)`, `Rigid` is a generic type parameter with bounds, and `Flex`
is an inference type variable (`Tvar`). `Ty_param.t` is just a name string
(`ty_param.ml:7`); `Ty_var.t` is an integer id (`ty_var.ml:3`).

### 1.2 Rows

`repr.ml:37-57`:

```
row =
  | Row_simple of row_simple          -- a concrete record shape
  | Row_splat of row_splat            -- an ordered list of things to merge

row_simple = { known : field_desc Label.Map.t ; unknown : base }

row_splat = { elems : row_splat_elem list }

row_splat_elem = Spread of base
```

A **simple row** is a finite map from known labels to field descriptors, plus an
`unknown` base standing for every label not in the map. The `unknown` is always the
type of an *optional* absent label. This is the important subtlety: a simple row does
not store a full field descriptor for the tail, only a base, because a required unknown
field would demand that every absent label be present, which is uninhabited. So the tail
projects to `Field_desc.opt unknown` (`repr.ml:626-630`). This is exactly Hack's
open-shape tail (`shape(..., ...)`): `unknown = Top` is a fully open shape, `unknown =
Bottom` is a closed shape.

A **splat** is an ordered list of spread elements. Each element is `Spread base`. If the
base is `Shape (Row_simple s)` the element is an inline row; if it is `Rigid a` it is a
spread of a generic parameter; if it is `Bottom` it is the bottom-row contribution
(`repr.ml:55-57`). Order matters: merge is rightmost-wins (Section 2).

### 1.3 Field descriptors and requiredness

`repr.ml:64-72`:

```
field_desc = { req : field_req ; base : base }
field_req  = Req | Opt
```

A field descriptor is a **product of two lattices**:

- **requiredness** is a presence lower bound. `Req` means the field is definitely
  present; `Opt` means it may be absent. The order is `Req < Opt` with `Req` as bottom
  and `Opt` as top (`repr.ml:75-88`, `field_req.ml` counterpart in
  `Field_req`, `repr.ml:438-463`).
- **base** is a type upper bound: the largest type the field's value can have.

Requiredness subtyping: a subfield must be at least as present as the superfield, so
`sub.req <= super.req`, i.e. `Req <= Opt` (`Field_req.lteq`, `repr.ml:449-454`). A
required field can be supplied where an optional one is expected, not the reverse. In
Hack terms, a required shape field is a subtype of the same field marked optional.

The pairing of presence and type as two independent coordinates is the key design
choice. TypeScript conflates "optional" with "`| undefined`", which is a known bug
source; keeping presence separate from type makes merge a clean product operation
(Section 2.3) and makes the corner reasoning for parameters clean (Section 4).

### 1.4 Distinguished rows

- `Row.Simple.top` (`repr.ml:596`): all fields optional at `Top`. The largest row.
- `Row.Simple.empty` (`repr.ml:601`): no known fields, `unknown = Bottom`. This is the
  **merge identity**, not bottom. It is not uninhabited, because `Opt Bottom` is
  satisfied by an absent field.
- `Row.bot` (`repr.ml:845`): the bottom row, represented as `Row_splat [Spread Bottom]`.
  It shape-wraps to `Base.Bottom`.

Note the asymmetry: a simple row is never bottom (`is_bot_row`, `repr.ml:229-232`);
bottom-ness lives only in `Base.Bottom` and in a splat that carries a `Spread Bottom`.

---

## 2. The merge algebra

Merge is the semantic content of spread: it collapses a splat (an ordered list of rows)
into one simple row, rightmost-wins and non-commutative. It is a monoid with
`Row.Simple.empty` as identity.

### 2.1 Requiredness meet and join

`repr.ml:75-88`. Two-point lattice, `Req` bottom, `Opt` top:

- `meet_field_req`: `Req` if either is `Req`, else `Opt`.
- `join_field_req`: `Opt` if either is `Opt`, else `Req`.

These are exercised exhaustively (only four inputs) in `field_req_tests`
(`test_splat.ml:592-624`).

### 2.2 Union, meet, join on base types

**Union** (`union_base`, `repr.ml:105-111`) is the join for base types. It is kept as a
first-class `Union` constructor and left-associated so that canonicalization is
idempotent. Crucially Splat does **not** distribute a union into a shape:
`Shape r1 | Shape r2` is never rewritten to `Shape (r1 join r2)`. That rewrite is only
denotation-preserving when the two rows differ in at most one field; with two or more
differing fields it invents cross terms and becomes unsound. The long comment at
`repr.ml:90-104` gives the counterexample
`{x:Bool,y:Bool} | {x:Nat,y:Nat}` versus `{x:Bool|Nat, y:Bool|Nat}`. Keeping unions
first-class is why join never needs to distribute.

**Meet** (`meet_base`, `repr.ml:113-126`): there are no intersection types, so meet
distributes structurally: `Bottom` absorbs, `Top` is identity, two shapes meet
field-wise (`meet_row`), a union meets by the distributive law
`(a|b) & t = (a&t)|(b&t)`, equal prims meet to themselves, unequal prims and a
prim-vs-shape meet to `Bottom`. Meeting with a `Flex` or `Rigid` leaf is unrepresentable
(no intersection type is available to express it) and raises; the caller must solve or
instantiate the variable first.

**Meet and join of rows** (`meet_row_simple`, `join_row_simple`, `repr.ml:142-182`):
pointwise over labels using `Label.Map.merge`, with an absent label filled in from the
row's `unknown` at `Opt`. Both are only defined on simple rows; a splat that still
carries a variable or parameter has no single-row meet or join and raises. Callers
canonicalize (which resolves solved splats to simple rows) first.

`Base.meet` and `Base.join` (`repr.ml:406-416`) canonicalize inputs and output, because
meeting two shapes can produce an uninhabited shape (a field meets to `Req Bottom`) that
must collapse to `Bottom` to keep `is_bot` correct.

These laws are checked as lattice properties against a brute oracle in `algebra_props`
(`test_splat.ml:655-808`): commutativity, associativity, idempotence, absorption,
identities, and "meet is the greatest lower bound" / "join is the least upper bound".

### 2.3 Merge on field descriptors (rightmost-wins)

This is the heart of spread. `merge_field_desc` (`repr.ml:184-191`):

```
merge ~left ~right =
  match right.req with
  | Req -> right                                    -- required right overrides fully
  | Opt -> { req = meet left.req Opt ; base = union left.base right.base }
```

Read it as: the right operand is the later spread. If the right field is **required**,
it completely overrides the left (it is definitely present with its own type). If the
right field is **optional**, the merged field's presence follows the left field
(`meet left.req Opt = left.req`) and its type is the **union** of both, because at
runtime the value is either the left one (right absent) or the right one (right present).

This is structurally identical to TypeScript's object-spread rule and to Hack's shape
spread: a required later key wins, an optional later key unions with the earlier type and
keeps the earlier presence. Because presence is a separate coordinate here, there is no
need for TypeScript's `\ undefined` workaround.

`Row.Simple.empty` (`Opt Bottom` everywhere) is the two-sided identity: merging it in
changes nothing (`meet left.req Opt = left.req`, `union base Bottom = base`). Verified as
`Field_desc.merge identity (opt bottom)` and `Simple.merge identity (empty row)`
(`test_splat.ml:735-807`).

### 2.4 Merge on simple rows

`merge_row_simple` (`repr.ml:193-209`): merge the `known` maps label-wise with
`merge_field_desc`, filling an absent label from that side's `unknown` at `Opt`, and set
the merged `unknown` to the union of the two `unknown` bases (both are optional, so the
union of bases is right).

### 2.5 Projection through a splat

Merging an entire splat is done lazily, one label at a time, by `Splat.proj`
(`repr.ml:792-816`). To project label `l`:

1. Start from the **rightmost** element's field at `l` (`Splat_elem.proj`,
   `repr.ml:682-690`: a rigid parameter reads its corner assignment from the passed
   `fld_assignment` map; `Bottom` yields `Req Bottom`; an inline simple row projects
   normally).
2. Walk leftward, folding with `merge_field_desc ~left ~right`.
3. **Short-circuit on `Req`** (`proj_help`, `repr.ml:792-804`): once the accumulated
   field is `Req`, everything further left is masked and cannot contribute, so stop.

The `None` label case projects the tail (`unknown`), always optional.

### 2.6 Canonicalization

`canon_base` / `canon_row` / `canon_row_splat` (`repr.ml:284-347`) put a type into a
normal form so that predicates like `is_bot` and structural equality are meaningful. The
important moves:

- A `Shape` whose row is bottom collapses to `Bottom` (`repr.ml:288-290`).
- A splat with a single concrete spread collapses to that simple row
  (`repr.ml:299-305`, `row_splat` at `repr.ml:260-267`).
- A `Spread (Shape (Row_splat ...))` is flattened: the inner splat's elements are
  spliced into the parent (`collapse_spread`, `repr.ml:312-316`).
- Adjacent concrete spreads are eagerly merged (`canon_row_splat_help`,
  `repr.ml:336-347`, using `merge_splat_elem_opt`, `repr.ml:250-258`). Two spreads merge
  only when both are concrete inline rows; an opaque `Rigid`/`Flex` spread or a
  `Spread Bottom` blocks the merge and stays a separate element.

Canonicalization is idempotent and denotation-preserving, checked in
`test_splat.ml:756-765` and (for the new spread paths) `canon_spread_tests`
(`test_splat.ml:1210-1233`). After canon, "is this row bottom" is decidable by inspecting
for a `Spread Bottom` (`is_bot_row`, `repr.ml:229-232`).

`tell_subtype` / `tell_subrow` (`splat.ml:955-970`) canonicalize their inputs once at the
top; the recursion below assumes it only ever sees canonical (flat) splats.

---

## 3. Subtyping: base types

`subtype_base ~sub ~super env` (`splat.ml:169-223`) returns an updated env and a residual
`Prop.t` (a proposition over the remaining flex variables; Section 5). When there are no
flex variables the residual is either `valid` or `invalid`.

**Rule order matters.** The comment at `splat.ml:169-174` fixes it. The cases, in order:

1. `Bottom <: _` is valid; `_ <: Top` is valid (`splat.ml:176-179`).
2. Any `Flex` on either side records a residual atom and stops (`splat.ml:181`). A flex
   sub against a union is recorded as a single upper bound, **not** split into a
   disjunction (splitting would be incomplete). Solving happens later, in dispatch.
3. A **sub-union** is conjunctive and exact: `(a|b) <: t` iff `a <: t` and `b <: t`
   (`splat.ml:183-186`).
4. A **super-union** distributes disjunctively: `t <: (a|b)` becomes `t <: a` or `t <: b`
   (`splat.ml:187-190`). This is checked after the sub-union and flex cases so that the
   disjunction never hides a variable that should have been recorded.
5. **Rigid parameters (F-sub)**: reflexivity, then reduce through the bound
   (Section 4.1).
6. Eager ground rejections: `_ <: Bottom` for non-bottom, `Top <: _` for non-top, prim
   mismatches, prim-vs-shape (`splat.ml:204-221`).
7. `Shape sub <: Shape super` reduces to `subrow` (`splat.ml:223`).

The ground fragment of this relation is checked to be sound and complete against an
independent brute oracle (`gsub`) over 25000 random pairs
(`ground differential: check = brute`, `test_splat.ml:449-455`), plus reflexivity,
bottom-least, top-greatest, transitivity, and the lattice laws.

---

## 4. Subtyping: the type-parameter cases

This section is the focus. It covers two mechanisms: the **F-sub rule** for a parameter
at base position, and the **corner decision procedure** for parameters spread into a row.

### 4.1 F-sub: a rigid parameter at base position

`splat.ml:191-223`. A `Rigid a` is a generic with an interval `[lower, upper]` of base
types recorded in `env.ty_param` (`Env.Ty_param_env`, `env.ml:4-80`). The rule:

- `Rigid a <: Rigid b` with `a = b`: reflexivity, valid (`splat.ml:197`).
- `Rigid a <: T`: reduce through `a`'s **upper** bound: `a <: upper(a) <: T`, so check
  `upper(a) <: T` (`splat.ml:198-200`).
- `T <: Rigid b`: reduce through `b`'s **lower** bound: `T <: lower(b) <: b`, so check
  `T <: lower(b)` (`splat.ml:201-203`).
- `Rigid a <: Rigid b` distinct: chains both, reducing to `upper(a) <: lower(b)`.

This is standard bounded-quantification F-sub. Reflexivity is tried first so a parameter
whose bounds are loose is still a subtype of itself. Tests: `fsub_tests`
(`test_splat.ml:824-838`), including the distinct-parameter chain.

Hack already implements exactly this for generic parameters with `as`/`super` bounds. The
only new thing is that a parameter can also appear at **spread** position, which is the
next subsection.

### 4.2 Why parameters spread into a row need corners

Consider a subrow query where a parameter is spread into one or both sides:

```
|{x: Req Bool}, ...T|  <:  |...T|     with T in [ {x: Req Nat} , {x: Opt Top} ]
```

For this to hold it must hold **for every instantiation** of `T` inside its bound
interval. `T`'s field at `x` is a field descriptor somewhere in the box between the
lower bound `Req Nat` and the upper bound `Opt Top`. Because the field descriptor is a
product of two lattices (requiredness and type), that box has up to **four corners**:

```
1) Req Nat   (lower bound)
2) Opt Nat   (top requiredness, bottom type)
3) Req Top   (bottom requiredness, top type)
4) Opt Top   (upper bound)
```

The worked example in `repr.ml:513-568` shows that checking only the two bounds (corners
1 and 4) reaches the wrong answer: both bounds pass, but corner 2 (`Opt Nat`) makes the
proposition false. So in general all four corners of the box must be checked.

The justification for corners being sufficient (rather than checking the whole
continuum) is monotonicity: `<:` is monotone in each field-descriptor coordinate, so the
worst case for `L <: R` sits at an extreme of each parameter's box. That is why a
parameter appearing on **one side only** collapses to a single corner (Section 4.5), and
a parameter on **both sides** needs the full box (up to four).

`Field_desc.corners ~lower ~upper` (`repr.ml:569-577`):

```
corners ~lower ~upper =
  match lower.req, upper.req with
  | Req, Opt -> [ lower ; upper ; {lower with Opt} ; {upper with Req} ]   -- 4
  | Req, Req | Opt, Opt -> [ lower ; upper ]                              -- 2
  | Opt, Req -> []                                                        -- ill-formed
```

The four-corner case only arises when the lower bound is `Req` and the upper is `Opt`
(a genuinely two-dimensional box). Tests: `corners_tests` (`test_splat.ml:627-652`) and
the property that every corner lies in the `[meet, join]` box
(`test_splat.ml:744-754`).

### 4.3 The per-label decision procedure

`subrow_splat` (`splat.ml:248-260`) reduces a splat-vs-splat (or splat-vs-simple) subrow
to a conjunction of per-label subfield checks, one per label in `subrow_label_set`
(Section 4.7). Each label is handled by `subrow_splat_at` (`splat.ml:319-396`), which is
the interesting procedure. At one label:

1. **Restrict to live parameters** (`splat.ml:322-324`). Only parameters that could
   still contribute to this label matter. `Row.live_spread_at` (`repr.ml:773-790`) walks
   a splat from the right and collects rigid parameters that are not masked by a `Req`
   field to their right. A `Req` inline field, or a `Spread Bottom`, masks everything to
   its left (rightmost-wins). Parameters to the left of such a required field cannot
   affect the merged field at this label, so they are dropped from the enumeration. This
   is condition (2) in the design comment at `splat.ml:271-277`.

2. **Topologically sort** the live parameters (`splat.ml:326-332`). Parameters can appear
   in each other's bounds (a parameter `Q` whose bound spreads `P`). The bound projection
   of one parameter reads the corner assignment of the parameters in its bound, so those
   must be assigned first. `Env.Ty_param_env.closure` (`env.ml:46-56`) pulls in every
   parameter reachable through bounds (so a parameter that lives *only* inside another's
   bound is included), and `Env.Ty_param_env.topo` (`env.ml:58-79`) orders them so
   dependencies come first.

3. **Compute the dependency set** `depended_on` (`splat.ml:335-339`): parameters that
   appear in some other parameter's bound. A parameter not in this set is "free": its
   assignment does not feed any other parameter's projection, so its corners can often be
   pruned (Section 4.5).

4. **Enumerate corner assignments** in topological order (`loop_params` /
   `loop_assignments`, `splat.ml:365-395`). For each parameter, `corners_for`
   (Section 4.5) returns the corner field descriptors to try; the loop takes the product
   over all live parameters, extending the `fld_assignment : field_desc Ty_param.Map.t`
   incrementally. Because the order is topological, when a parameter's corners are
   computed the parameters in its bound already have assignments in the map.

5. **At a complete assignment**, project both rows at the label with that assignment
   (`Row.proj`, which runs the merge of Section 2.5 with the parameters resolved to their
   corner fields) and emit the subfield check (`splat.ml:377-382`, `subfield` at
   `splat.ml:863-874`).

The whole thing is a conjunction: the subrow holds iff the subfield check passes at every
label and every corner assignment. This is co-assignment exponential in the number of
live, dependent parameters, but in practice the count is small and the shortcuts below
prune it hard.

### 4.4 Field bounds of a parameter at a label

`Row_help.field_bounds` (`splat.ml:67-77`): to get a parameter's `[lower, upper]` box at
a label, read the parameter's bound shapes from `env.ty_param`, take the row of each
bound (`row_of_base`, `splat.ml:23-29`: a shape's row, or the bottom row for `Bottom`),
and project each at the label under the current partial assignment (so a parameter whose
bound itself spreads another parameter is resolved through that parameter's already-fixed
corner). The result is the field-descriptor interval whose corners drive the enumeration.

### 4.5 The corner shortcuts (`corners_for`)

`corners_for` (`splat.ml:342-361`) applies the four optimizations described in the design
comment at `splat.ml:280-317`. Let a parameter be `free` (not in `depended_on`),
`in_sub` (live in the sub-row at this label), `in_super` (live in the super-row):

- **(i) sub side only** (`free && in_sub && not in_super`): use `[upper]` only. A
  parameter appearing only in the sub-row contributes only to the left of `<:`; the
  hardest case is the one that maximizes the sub-row, which is the parameter's upper
  bound.
- **(i) super side only** (`free && not in_sub && in_super`): use `[lower]` only.
  Symmetric: the hardest case minimizes the super-row, the lower bound.
- **both sides** (`free && in_sub && in_super`): look at masking (Section 4.6):
  - **(ii) masked in sub** (`masking_sub = Masked`): the parameter cannot contribute to
    the sub-row (something to its right always overrides it there), so it acts as a
    super-only parameter: use `[lower]`.
  - **(iii) masked in super** (`masking_super = Masked`): symmetric, super side is fixed,
    acts as sub-only: use `[upper]`.
  - **(iv) unmasked in super with optional lower** (`masking_super = Unmasked` and
    `lower.req = Opt`): use `[lower]`. When the parameter can never be masked on the super
    side and its lower requiredness is already `Opt`, the field's *presence* is settled
    (the merge stays `Opt`), so only the *type* matters, and the type that is hardest for
    `<:` is the one that grows the super side least, the lower bound. The proposition has
    the shape `(U | t) <: (V | t)` where `t` is this field's type, and a free `t` is
    worst-cased at its lower bound.
  - otherwise: the full `corners ~lower ~upper` (up to four).
- **not free**: the full `corners`. A depended-on parameter cannot be pruned, because its
  choice changes another parameter's projected bounds.

These shortcuts are what keep the enumeration cheap: a parameter that occurs on only one
side, or is masked, collapses from four corners to one.

### 4.6 Masking

`Row_help.masking_of` / `masking_of_splat` (`splat.ml:99-165`). At a label and a partial
assignment, a parameter within a row is:

- **Masked**: some parameter to its right is `Req` in its **upper** bound at this label,
  so that rightward parameter always supplies the field and overrides everything to the
  left. The target cannot contribute.
- **Unmasked**: every parameter to its right is `Opt` in its **lower** bound, so nothing
  to the right can override it.
- **Unknown**: neither holds (some rightward parameter has `Req` lower but `Opt` upper, so
  we cannot tell), or the target does not occur in this row. Unknown is the pessimistic
  answer and blocks the single-corner shortcuts, falling back to full corners.

The key subtlety (`splat.ml:133-144`): whether a rightward parameter `r` masks the target
is decided by `r`'s **own** bounds at this label, not the target's. If `r`'s upper is
`Req` it masks; if `r`'s lower is `Req` but upper `Opt` the answer is Unknown; if `r` is
`Opt` at both bounds it does not mask. This is regression-tested by
`masking: rightward param's bounds decide` (`test_splat.ml:970-976`) and its mirror
(`test_splat.ml:980-988`).

### 4.7 Discovering labels that live only in bounds

`Row.label_set` of a splat of bare parameter spreads is empty (a `Spread (Rigid a)` has
no inline labels). But a label can live only inside a parameter's bound, and it is still
a real per-field obligation. `Row_help.bound_label_set` (`splat.ml:38-51`) collects the
labels of every bound of every parameter mentioned, taking the transitive closure so a
label buried in a nested parameter's bound is found too. `subrow_label_set`
(`splat.ml:55-60`) unions the inline labels of both rows with these bound labels, and the
per-label loop visits all of them plus the `None` (unknown-tail) pseudo-label
(`subrow_labels`, `splat.ml:62-64`). Missing this would silently skip a field and be
unsound; tested by `bound-only label is discovered` (`test_splat.ml:989-993`).

### 4.8 Nested parameters

A parameter `Q` whose bound spreads `P` (so `P` is reachable only through `Q`'s bound) is
"the hard case". The transitive `closure` (Section 4.3 step 2) pulls `P` into the
enumeration, `topo` orders `P` before `Q`, and `field_bounds` projects `Q`'s bound with
`P` already assigned to a corner. Without the closure the engine would raise. This is
covered by `nested_tests` and the `nested-param differential` property over 20000 cases
(`test_splat.ml:1120-1158`), whose oracle universally quantifies `P` with `Q` determined
by `P`.

### 4.9 The differential oracle for parameters

`rigid-param differential: check = forall-instantiation` (`test_splat.ml:1023-1035`,
generator at `test_splat.ml:840-932`) is the strongest correctness check on the corner
machinery. It places parameters `P`, `Q` at spread position with random shape-bound
boxes, then compares the engine's decision against a **true denotational oracle** that
enumerates every instantiation of `P` and `Q` within their boxes and checks the concrete
subrow at each. This exercises corner enumeration, the masking shortcut, bound-label
discovery, and single-side drops, over 20000 random cases. The engine agrees exactly:
the corner procedure decides the semantic (universally quantified) subrow.

---

## 5. Subtyping: simple rows

`subrow_simple` (`splat.ml:833-856`) is the ground case: both sides are concrete simple
rows. It checks the subfield relation at every known label of either side plus the `None`
tail. `subfield` (`splat.ml:863-874`): a subfield holds iff `sub.req <= super.req`
(presence) and `sub.base <: super.base` (type), the two coordinates checked
independently. Projection at an absent label uses the row's `unknown` at `Opt`
(`Simple.proj`, `repr.ml:626-630`).

This is the same shape-subtyping Hack already does for `shape(...)` types: required beats
optional, field types are checked covariantly, and the open tail governs unknown fields.

---

## 6. Inference

Inference has two independent axes: **flex variables** (unification variables at base
position) and **spread variables** (a flex variable spread into row position). Both are
driven through a residual proposition.

### 6.1 The residual proposition

`prop.ml`. `subtype_base` and friends do not decide immediately when a flex variable is
involved; they emit a `Prop.t`:

```
atom = Subtype_base { sub ; super } | Subfield { label ; sub ; super }
t    = Atom of atom | Conj of t list | Disj of { failures ; props }
```

`valid = Conj []`; `invalid f = Disj { failures = [f]; props = [] }`. Smart constructors
`conj` / `disj` (`prop.ml:41-81`) keep the tree normalized: `conj` drops valids and folds
nested conjunctions, `disj` collects alternatives. `is_invalid` is "a `Disj` with no
surviving alternatives" (`prop.ml:29-33`); `is_valid` is "the empty `Conj`". A query is
**accepted** when its final proposition is not invalid (`test_splat.ml:16-18`).

### 6.2 Flex variables and dispatch

After building the residual, `tell_prop` (`splat.ml:883-952`) walks it, records each
variable constraint in the env, and for every genuinely new bound generates the implied
transitive constraint and dispatches that in turn. The recursion terminates because
implications are generated **only for newly recorded bounds**.

- `tell_atom_base` (`splat.ml:921-929`): a `Flex <: Flex` records an upper on the left
  variable and a lower on the right; a one-sided flex records one bound; a ground atom is
  re-checked by `subtype_base`.
- `record_ty_upper` (`splat.ml:931-940`): add `super` as an upper bound of `var_sub`; if
  new, for each existing lower bound `l` of the variable emit `l <: super` (transitivity
  through the variable) and dispatch.
- `record_ty_lower` (`splat.ml:942-952`): the mirror.

Bounds are stored per variable in `Env.Ty_var_env` (`env.ml:82-110`) as lists of uppers
and lowers. This is the standard "constraint graph, propagate on new edge" approach and
matches how Hack's `Typing_subtype` records `Tvar` bounds and closes them transitively.
The transitive-closure behaviour is checked in `flex transitive closure`
(`test_splat.ml:427-431`) and the full soundness-and-completeness closure over 25000
cases in `flex differential: check = exists-witness` (`test_splat.ml:483-493`).

### 6.3 Solving a flex variable

`Env.solve_ty_var` (`env.ml:143-152`): a variable's current solution is read **only** from
bounds already discharged into the env, never from undischarged residual atoms. With
lower bounds present, the solution is the join of the lowers (the least value satisfying
them); with only uppers, the meet of the uppers; with neither, `Bottom`. Bounds are
resolved recursively before combining (`resolve_base`, `env.ml:207-214`), so meet and
join only see ground types. Termination relies on the bound graph being acyclic. The
recovered solution is checked to be a real witness in `solution is a witness`
(`test_splat.ml:497-507`).

### 6.4 Spread variables: overview

A spread variable is a flex variable in spread position (`Spread (Flex v)`), standing for
an unknown row to be merged in. The dispatch in `subrow` (`splat.ml:226-241`) branches on
which sides carry spread variables:

- neither side: ground splat, go to `subrow_splat` (Section 4).
- sub side only: `subrow_infer_sub` (Section 6.5).
- super side only: `subrow_infer_super` (Section 6.6).
- both sides: `subrow_infer_sub_super` (Section 6.7).

For inference we want at most one spread variable per side; `topo_spread_vars`
(`repr.ml:737-748`) picks an order and `rightmost_spread_var` (`repr.ml:759-768`) is the
one to retain when eliminating (Section 6.8).

### 6.5 A spread variable in the sub-row

`subrow_infer_sub` / `subrow_infer_sub_part` / `subrow_infer_sub_part_at`
(`splat.ml:402-582`). For a variable `?p` in the sub-row, partition the sub-row at `?p`
into `pre` (left of `?p`) and `post` (right of `?p`). Resolve every *other* variable to
its current solution (`solve_all_except`, `splat.ml:11-15`), leaving `?p` symbolic. Then,
per label, build the field descriptor that `?p` must have and accumulate a residual
constraint `?p <: shape({those fields})` (`splat.ml:459-463`).

At a single label (`subrow_infer_sub_part_at`, `splat.ml:475-582`) the logic mirrors the
merge:

- If `post` **requires** the field at every corner assignment, `?p` is overwritten
  everywhere (masked to its right), so its upper bound is `Top` and we just check
  `post <: super` per assignment.
- Otherwise `?p` contributes. Its requiredness is forced to `Req` when the field is `Req`
  in super and `pre` cannot supply it, or when `pre`'s type does not fit super so `?p`
  must be `Req` to mask the offending `pre` field. This is the `must_req` disjunction at
  `splat.ml:522-546`. The merged sub type at this label is the union of the fresh variable
  for `?p`, `post`, and (unless `?p` is `Req`) `pre`, checked against super.

The unknown-tail label (`None`) never forces `Req`, because a required unknown is
unrepresentable (`splat.ml:526-531`).

The sub-side masking fix at `splat.ml:536-545` (force `?p` to `Req` to mask a
type-violating neighbour) is what makes the single-row-variable relation exactly complete;
without it there was a residual over-reject. The round trip is checked in
`row differential (1 var): check_row = exists-witness` over 20000 cases
(`test_splat.ml:508-520`) and `row solving: recovered solution is a witness`
(`test_splat.ml:527-538`).

### 6.6 A spread variable in the super-row

`subrow_infer_super` and helpers (`splat.ml:585-765`) are the dual. For `?p` in the
super-row, per label build the field `?p` must have as a **lower** bound and accumulate
`shape({those fields}) <: ?p`. The requiredness of `?p`'s field mirrors the sub-row: `Opt`
if some live sub field is `Opt` (the merge must then stay `Opt`), else `Req`. An
unsatisfiable case is a sub field that is `Opt` while super `pre` is `Req` (the merge is
forced `Req`, and `Opt` is not `<= Req`), reported invalid at `splat.ml:731-743`.

### 6.7 Spread variables on both sides

`subrow_infer_sub_super` (`splat.ml:805-829`):

- **Exactly one variable on each side**: couple them through a fresh intermediary row
  `mid` (`subrow_infer_couple`, `splat.ml:779-803`). Build a fresh field per label in
  `mid`, check `sub <: mid` (which bounds `?p_sub` from above) and `mid <: super` (which
  bounds `?p_super` from below); `mid`'s fresh variables link the two so the sub side's
  contribution flows through to the super side. Sound by transitivity
  `sub <: mid <: super`. `mid`'s known fields are fixed `Req` (measured to over-reject
  much less than `Opt`), and its unknown is a fresh opt-only base. This keeps the
  cross-side coupling that the naive elimination would drop.
- **More than one variable on a side**: fall back to the **decoupled** solver
  (`splat.ml:816-829`). Resolve the other side's variables to their current solutions
  (an unsolved one becomes the empty row) to make it concrete, then solve this side
  against it. Sound, but the cross-side coupling is dropped, so this over-rejects. The
  two-variable case is checked sound-only in
  `row differential (2 vars): SOUND (accept implies witness)` (`test_splat.ml:539-549`),
  and the `DIAG` harness (`test_splat.ml:1309-1377`) quantifies the over-reject rate.

### 6.8 Resolving a spread variable to a row

`Env.resolve_spread_var` (`env.ml:170-196`): a spread variable resolves to its current
solution **as a row**. The bounds are combined at the **row** level: `Row.join` of the
lower shapes (covariant) or `Row.meet` of the upper shapes (contravariant), not via
`Base.join` / `Base.meet`. This matters because base join does not distribute into shapes,
so combining two lower shapes with `Base.join` would produce an ill-formed
`Spread (Union (Shape, Shape))`; the row join is the correct single-row bound. An unbounded
spread variable resolves to the **empty row** (the merge identity, contributing nothing),
not bottom. Tested by `spread var with 2 distinct lowers stays a single row`
(`test_splat.ml:996-1019`).

`resolve_row` / `resolve_base` (`env.ml:156-214`) substitute every flex variable by its
current solution recursively, with an optional `except` to leave the one variable being
solved untouched.

---

## 7. Known incompleteness (deliberate)

These are the facts the engine drops to stay sound and decidable. Each is documented in
the source with a counterexample.

1. **No shape/union distribution** (`repr.ml:90-104`). `Shape r1 | Shape r2` is never
   folded to `Shape (r1 join r2)`. Sound only when the rows differ in at most one field;
   otherwise it invents cross terms. Consequence: `{x:Bool} | {x:Nat}` is not recognized
   as a subtype of `{x: Bool|Nat}`.

2. **Both-sides spread coupling** (`splat.ml:769-778`). With one spread variable on each
   side the intermediary `mid` fixes its known fields to `Req`, which over-rejects some
   satisfiable queries (the "keep-both requiredness" gap). With more than one variable on
   a side the coupling is dropped entirely (Section 6.7). Both are sound.

3. **Meet with a variable or parameter raises** (`repr.ml:122-125`). There are no
   intersection types, so an unrepresentable meet is a hard error; the caller must solve
   or instantiate first.

None of these affect soundness. The ground and single-spread-variable fragments are
complete (Sections 4.9, 6.5); the multi-variable both-sides fragment is sound only.

---

## 8. Test strategy

The tests are the specification of correctness and are worth porting as differentials:

- **Ground differential** (`test_splat.ml:449-455`): engine vs brute per-field oracle
  `gsub`, 25000 cases. Sound and complete on ground types.
- **Flex differential** (`test_splat.ml:483-493`): a variable query is accepted iff some
  chain assignment satisfies it, 25000 cases.
- **Solution is a witness** (`test_splat.ml:497-507`): the solved value really satisfies
  the query.
- **Row differential, 1 var** (`test_splat.ml:508-520`): the single-spread-variable
  relation is a full iff.
- **Rigid-param differential** (`test_splat.ml:1023-1035`): the corner procedure vs a
  universally quantified denotational oracle over parameter instantiations, 20000 cases.
  This is the strongest check on the type-parameter machinery.
- **Nested-param differential** (`test_splat.ml:1146-1158`): the transitive closure case.
- **Row differential, 2 vars** (`test_splat.ml:539-549`): sound only; the `DIAG` harness
  quantifies the over-reject.
- **Algebra laws** (`test_splat.ml:655-808`): merge, meet, join, canon, corners against
  the lattice laws and the brute order oracle.

The pattern to reuse: an independent brute oracle that enumerates the finite witness
universe (ground types, or all instantiations of a bounded parameter) and a
`check = oracle` differential. That is what ties the corner procedure to the semantic
subrow relation.
