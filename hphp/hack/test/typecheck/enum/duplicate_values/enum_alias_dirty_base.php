<?hh

// These aliasing enums name a base that is not a clean, duplicate-free enum --
// it has duplicate values, an unevaluable member, or is not an enum at all -- so
// the alias is not exempt and its members are uncheckable.

// Aliasing an enum that itself has duplicate values: the base is not clean.
enum DirtyBase: int as int {
  D = 1;
  E = 1; // duplicate -> DirtyBase is flagged, and is not "clean"
}

enum AliasOfDirty: int as int {
  M = DirtyBase::D; // uncheckable (base not clean)
  N = DirtyBase::E; // uncheckable (base not clean)
}

// Accessing a constant on a non-enum class is never a clean alias.
class NotEnum {
  const int V = 1;
}

enum AliasOfClass: int as int {
  W = NotEnum::V; // uncheckable
}

// Two aliased members that share a value: the alias would carry the same
// collision, so it is not exempt. Here BaseHasUse::IB (inherited, 1) and
// BaseHasUse::HB (1) are both 1.
enum IncBase: int as int {
  IB = 1;
}

enum BaseHasUse: int as int {
  use IncBase; // brings in IB = 1
  HB = 1; // collides with the inherited IB = 1 -> BaseHasUse is flagged
}

enum AliasOfBaseHasUse: int as int {
  R = BaseHasUse::IB; // uncheckable (R and S alias the same value)
  S = BaseHasUse::HB; // uncheckable (R and S alias the same value)
}

// Naming an unevaluable member is uncheckable, even when the base's other
// members are concrete.
const int SOME_GLOBAL = 7;

enum PartlyUncheckableBase: int as int {
  PA = 1;
  PB = 2;
  PC = SOME_GLOBAL; // unevaluable -> PartlyUncheckableBase itself is flagged
}

enum AliasOfUncheckablePart: int as int {
  BA = PartlyUncheckableBase::PA;
  BB = PartlyUncheckableBase::PC; // uncheckable (PC has no evaluable value)
}

// `MEMBER = 'MEMBER'` is recorded as a label, whose value is the *aliased*
// member's name -- not the aliasing member's. Both members aliased here are the
// string "LP", so the alias is not exempt; resolving the label against the wrong
// name would miss that.
enum LabelBase: string as string {
  LP = 'LP'; // label form: value equals the member name
  LQ = 'LP'; // same value spelled out -> LabelBase is flagged
}

enum AliasOfLabelDup: string as string {
  LX = LabelBase::LP; // uncheckable (LX and LY both alias "LP")
  LY = LabelBase::LQ;
}
