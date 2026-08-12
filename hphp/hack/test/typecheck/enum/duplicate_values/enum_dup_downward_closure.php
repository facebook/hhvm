<?hh

// An enum that `use`s an enum which cannot be checked for duplicate values is
// itself uncheckable, and must opt out as a whole with
// `<<__AllowUncheckedEnumValues>>`. We deliberately do NOT check it only in part
// (e.g. flag its own duplicates while ignoring the used enum), because that
// would give a false sense of safety.

// Opted-out base with an intentional duplicate.
<<__AllowUncheckedEnumValues>>
enum DcBase: int as int {
  A = 1;
  B = 1; // intentional duplicate, allowed by the attribute
}

// Uses an opted-out enum -> uncheckable as a whole. Its own C/D/Dup are not
// separately checked; the enum is asked to opt out too.
enum DcUser: int as int {
  use DcBase;
  C = 1;
  D = 2;
  Dup = 2;
}

class DcConst {
  const int K = 1;
}

// A base with a value that can't be statically evaluated -> flagged on its own
// definition (the root cause the user must fix or opt out).
enum DcComputedBase: int as int {
  E = DcConst::K; // constant reference, can't be checked
}

// Uses an enum whose value is unevaluable but which has NOT opted out. The error
// is reported on `DcComputedBase` itself, not duplicated here: only a used enum
// that actually opts out (like `DcBase` above) makes its includer uncheckable.
// Fixing or opting out `DcComputedBase` resolves it.
enum DcComputedUser: int as int {
  use DcComputedBase;
  F = 5;
}

// A base that can be checked: an includer is checked normally.
enum DcStrictBase: int as int {
  X = 10;
}

enum DcStrictUser: int as int {
  use DcStrictBase; // brings in X = 10
  Y = 10; // collides with DcStrictBase::X, both checkable -> flagged
}

// Opting out silences everything, even a real duplicate and a computed value.
<<__AllowUncheckedEnumValues>>
enum DcOptOutBoth: int as int {
  P = 1;
  Q = 1; // duplicate of P
  R = DcConst::K; // computed
}

// Using TWO distinct opted-out enums: uncheckable as a whole, and the error
// lists both used enums as reasons (deduped and sorted by name).
<<__AllowUncheckedEnumValues>>
enum DcOptOutA: int as int {
  Oa = 1;
}

<<__AllowUncheckedEnumValues>>
enum DcOptOutB: int as int {
  Ob = 2;
}

enum DcUsesTwoOptedOut: int as int {
  use DcOptOutA; // opted out -> uncheckable
  use DcOptOutB; // opted out -> uncheckable
  Z = 3;
}
