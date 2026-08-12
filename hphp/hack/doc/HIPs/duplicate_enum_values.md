# Feature Name: Reject duplicate enum member values
## Start Date: 2026-06-19
## Status: Draft

# Summary:

Make it a typecheck error for two members of the same enum to declare the same
value, including when one of the members comes from a `use`-included enum.

```
enum Color: int { Red = 1; Green = 1; Blue = 3; } // Green has the same value as Red
```

Today such enums typecheck cleanly ("No errors") and only fail at runtime:
`HH\BuiltinEnum::getNames()` builds a value-to-name map and throws an
`InvariantViolationException` when two members share a backing value, crashing
whatever calls it.

The check is gated behind the `check_duplicate_enum_values` `.hhconfig` option.
A core goal is to leave **no silent gaps**: every enum is either *fully checked*
or *explicitly opted out* via an attribute. An enum is opted out when it
intentionally aliases values, or when some of its values are *unverifiable* (a
constant reference, arithmetic, or another form the check cannot compare);
otherwise an unverifiable value is itself an error (see *User experience*).

# Feature motivation:

Duplicate enum values are a recurring source of production SEVs.
`HH\BuiltinEnum::getNames()` throws an `InvariantViolationException` when two
members share a backing value, and that exception crashes whatever calls it.
Recent incidents:

- [S565800](https://www.internalfb.com/intern/sevmanager/view/s/565800) — SEV2,
  137K+ affected users (Encrypted Backups, Messenger group-add, Workchat and IGD
  broken via a Trusted Caller enum).
- [S652374](https://www.internalfb.com/intern/sevmanager/view/s/652374),
  [S623413](https://www.internalfb.com/intern/sevmanager/view/s/623413),
  [S659583](https://www.internalfb.com/intern/sevmanager/view/s/659583) — SEV3s.

The blast radius is concentrated in logging/event listeners. Instagram's Falco
event listeners are the most frequent victims: hundreds of callsites under
`www/flib/logging/Falco/Listeners/` call `getNames()` to map client-reported int
values to strings, many via the unguarded direct-access pattern
`getNames()[$value]`. When the invariant fires, the whole listener crashes and
logging data for that event is silently dropped — sometimes for hours before
detection.

The root cause is almost always a **land-time collision**: two diffs land
concurrently, each adding an enum member with the same value. No merge conflict
is raised because the additions touch different lines, so neither author sees
the clash.

Existing tooling does not reliably prevent this: the current lint rule is
best-effort and has structural gaps (see *Prior art*), so duplicates still reach
production and fail late at runtime, where they are hard to attribute. The
codebase already carries accidental duplicates today (e.g. `CodecType`, whose
members are all `0`), which the check surfaces.

# User experience:

With the option on, a duplicate is a typecheck error. So that error-driven
codemods can rely on a consistent shape, the **primary position is the enum
declaration**, with each colliding member reported as a **secondary position**:

```
enum Color: int { Red = 1; Green = 1; Blue = 3; }
//   ^ Enum `Color` declares the value `1` for more than one member
//                 --- Red          ----- Green   (secondary positions)
```

(Anchoring the error at the enum also handles a clash introduced purely by
`use`-inclusion, where no local member is involved and pointing inside an unowned
file would be misleading.)

Three situations require an explicit opt-out attribute; otherwise they are
errors:

1. **Intentional aliasing** — the enum deliberately gives two members the same
   value.
2. **Unverifiable values** — a member's value is something the check cannot
   compare (a constant reference, arithmetic, or a non-UTF-8 string literal; see
   *Implementation details*). Skipping these silently would leave holes: a
   computed value can collide with a checkable one in an enum that `use`s it and
   go undetected, e.g. `A = 1 << 1` in one enum and `B = 2` in an enum that
   includes it.
3. **Downward closure** — an enum that `use`-includes an opted-out enum inherits
   its unchecked/duplicate values, so it must itself carry the attribute. This
   keeps the exemption visible at every enum that relies on it rather than
   silently inherited (a `use` of an annotated enum should not quietly grant a
   false sense of safety).

```
<<__AllowDuplicateValues>>             // name under discussion -- see below
enum Legacy: int { A = 0; B = 0; }     // permitted

enum AlsoLegacy: int {
  use Legacy;                          // inherits a duplicate value, so
}                                      // AlsoLegacy must also be annotated, else error
```

# The opt-out attribute:

Because the attribute now permits more than literal duplicates — it also marks an
enum whose values are *unverifiable* — `<<__AllowDuplicateValues>>` is arguably
too narrow a name. A single attribute covering both cases is sufficient; the name
is open for the review (candidates: `<<__AllowDuplicateValues>>`,
`<<__AllowComputeAndDuplicateValues>>`, `<<__UncheckedEnumValues>>`). The
attribute is **downwards-closed**: a `use` of an annotated enum requires the
using enum to be annotated as well.

# IDE experience:

Surfaces as a standard typecheck diagnostic. No special IDE handling is required.

# Implementation details:

- *Recording.* The direct decl parser records each enum member's value on the
  class constant as a discriminated value — int (as `i64`), string, `nameof`, or
  `::class` — gated by a parser option, `include_enum_member_values`. A `tast`
  check then iterates an enum's folded constants and flags any value that repeats
  an earlier member's, plus (under the "no silent gaps" rule) any unverifiable
  member of a non-exempt enum. Because folded decls already merge `use`-included
  members, direct duplicates, local-vs-used clashes, and clashes between two
  included enums are all caught in one pass.
- *Canonicalization.* Values are normalized so equivalent literals compare equal
  (e.g. `0x10` == `16`, `'a'` == `"a"`). Only int/string literals, `nameof`, and
  `::class` are recorded; anything else (constant references, arithmetic) is
  unverifiable.
- *`nameof` vs `::class`.* These are recorded as **distinct** kinds and never
  collide with each other: at runtime `nameof C` is a string but `C::class` is a
  class pointer, so they are not equal. (Class-pointer work by @vmladenov /
  @dizzy may make `::class` ineligible as an enum value altogether, in which case
  this case disappears.)
- *Non-UTF-8 string literals.* Treated as unverifiable: a non-UTF-8 byte string
  has no lossless comparable canonical form, so recording it risks two distinct
  byte strings collapsing to the same key and being *falsely* flagged as
  duplicates.
- *Opt-out.* The attribute makes the check skip the enum and the parser drop its
  recorded values; it is downwards-closed (see above).
- *HHVM changes.* None required.
- *Corner cases.* Enum classes are out of scope (plain enums only).
- *Codemodding.* Existing duplicate-value and unverifiable-value enums must be
  fixed or annotated before the check is turned on repo-wide.

# Migration scale:

The "no silent gaps" rule means two populations need migrating before rollout:
enums with duplicate values, and enums with unverifiable values. To size this
(asked by reviewers), we will gather from a local run over WWW:

- the number of would-be duplicate-value errors, and the share that are in
  generated code;
- the number of enums with unverifiable/computed values (the additional
  migration "no silent gaps" implies);
- coverage: the fraction of enum members that are checkable (int/string/`nameof`/
  `::class`) vs unverifiable.

*(Numbers pending; to be filled in from a local `hh` run over WWW.)*

# Design rationale and alternatives:

- *Typecheck-time vs runtime.* The bug is severe and currently only fails at
  runtime; catching it statically is the whole point.
- *No silent gaps (error on unverifiable values rather than skip).* Silently
  skipping unverifiable values leaves holes — a computed value can collide with a
  checkable one in an enum that `use`s it, undetected. Requiring those enums to
  carry the opt-out makes every enum either checked or explicitly exempt. This is
  cheap to implement (the decl already records nothing for unverifiable members,
  so the check just flags such a member of a non-exempt enum) at the cost of a
  larger migration. It also answers "why should the attribute affect computed
  values at all": because computed/unverifiable values are now errors by default,
  not silently allowed — the attribute is exactly what permits them.
- *Canonical-value comparison vs full constant evaluation.* Comparing recorded
  canonical literals keeps the check simple and predictable. Resolving arbitrary
  constant expressions (e.g. `X = Foo::BAR` or arithmetic) was considered and
  dropped. Values are recorded by the direct decl parser -- a fast, syntactic,
  per-file pass that intentionally neither resolves references nor evaluates
  expressions. Full evaluation would mean building a constant evaluator inside
  that parser and resolving references across decls (looking up the referenced
  constant's own decl, often in another file). That cross-decl resolution also
  adds fanout: an enum member's recorded value would then depend on the
  referenced decl, so editing that decl would have to re-record the enum and
  re-run the check -- enlarging the decl's dependency footprint and the
  incremental-recheck cost. The "no silent gaps" rule lets us be safe without
  paying for evaluation: unverifiable values are surfaced (via the required
  attribute) rather than resolved.
- *Error, not warning.* A warning would be easy to ignore for a bug this likely
  to break at runtime.
- *Opt-out attribute vs always-on.* A few enums legitimately alias values or use
  unverifiable values; the attribute is a precise, visible escape hatch rather
  than disabling the check globally.

# Prior art:

- **Aurora lint rule `DUPLICATE_ENUM_VALUES`** (`DuplicateEnumValuesLintRule` in
  `www/flib/intern/aurora/lint_rules/ast_linters/`) already flags duplicate enum
  values, but as best-effort lint it has structural gaps that let collisions
  through: it does not run on generated files or on files too long / that fail to
  parse, and catching collisions introduced across files via enum `use`-inclusion
  has been a persistent struggle. The typecheck-level check closes these gaps —
  it reads the folded decl (including `use`-merged members) for every enum,
  generated or not, and reports before the duplicate can land.
- **Runtime-resilience proposal:**
  [RFC: Make Enum::getNames() resilient to duplicate values](https://fb.workplace.com/groups/hackforhiphop/permalink/31492867100335171/)
  proposes making `getNames()` tolerate duplicates (a fallback string) instead of
  throwing. That is complementary rather than competing: resilience limits the
  runtime blast radius, while this proposal prevents duplicates from landing at
  all — the two can coexist.
- **Prior discussion:**
  [Duplicate enum value - Hack Warning?](https://fb.workplace.com/groups/hackforhiphop/permalink/29663604556594777/).

# Drawbacks:

- Existing duplicate-value *and* unverifiable-value enums must be fixed or
  annotated before rollout (size TBD — see *Migration scale*).
- *Decl memory footprint.* Recording a value per enum member enlarges decls.
  Measured full-init peak memory increase is ~+1.3% (a point estimate, not
  statistically significant at 99%); ints are stored as an 8-byte `i64` rather
  than a string, and a member without a recorded value costs only an empty
  option slot.

# Unresolved questions:

- Rollout plan and the timeline for turning the check on by default.
- The final attribute name (see *The opt-out attribute*).

# Future possibilities:

- Resolve simple constant references to widen coverage, turning some
  currently-unverifiable values into checkable ones and shrinking the migration.
