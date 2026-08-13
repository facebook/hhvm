<?hh

// A pure alias -- an enum with no `use` inclusions whose members are all
// distinct constant accesses on a single duplicate-free enum -- has values as
// distinct as the enum it aliases, so it is exempt from the uncheckable-value
// check without needing the opt-out attribute. All aliases below are exempt.

enum AliasBase: int as int {
  A = 1;
  B = 2;
  C = 3;
}

// Pure alias of a clean enum: exempt, no error.
enum AliasClean: int as int {
  X = AliasBase::A;
  Y = AliasBase::B;
  Z = AliasBase::C;
}

// A base with `use` inclusions is fine as long as the aliased members have
// distinct values -- inherited members carry their values through the fold.
enum IncCleanBase: int as int {
  ICA = 10;
}

enum BaseHasCleanUse: int as int {
  use IncCleanBase; // brings in ICA = 10
  HCB = 20;
}

enum AliasOfBaseHasCleanUse: int as int {
  CA = BaseHasCleanUse::ICA; // exempt: 10 and 20 are distinct
  CB = BaseHasCleanUse::HCB;
}

// An unevaluable member *elsewhere* in the base does not disqualify an alias
// that does not name it. Only the aliased members are checked, so a single
// unevaluable member in a widely-aliased enum does not flag the enums that
// alias its other, concrete members. (PartlyUncheckableBase is flagged for `PC`.)
const int SOME_GLOBAL = 7;

enum PartlyUncheckableBase: int as int {
  PA = 1;
  PB = 2;
  PC = SOME_GLOBAL; // unevaluable -> PartlyUncheckableBase itself is flagged
}

enum AliasOfConcretePart: int as int {
  AA = PartlyUncheckableBase::PA; // exempt: PA and PB are concrete and distinct
  AB = PartlyUncheckableBase::PB;
}
