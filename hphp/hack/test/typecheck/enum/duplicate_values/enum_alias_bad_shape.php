<?hh

// These aliasing enums are NOT pure 1:1 aliases of a single clean enum -- the
// shape of the aliasing enum itself disqualifies the exemption -- so their
// constant-access members are uncheckable.

enum AliasBase: int as int {
  A = 1;
  B = 2;
  C = 3;
}

// Mixing an access with a literal: not all members are accesses, so not a pure
// alias -> the access is uncheckable.
enum AliasMixed: int as int {
  P = AliasBase::A; // uncheckable
  Q = 5;
}

// Accesses spread across two different enums: not a single base -> not exempt.
enum OtherBase: int as int {
  F = 10;
  G = 20;
}

enum AliasTwoEnums: int as int {
  H = AliasBase::A; // uncheckable
  I = OtherBase::F; // uncheckable
}

// The same constant accessed twice: the accessed members are not distinct, so
// the alias could hide a duplicate and is not exempt.
enum AliasSameTwice: int as int {
  J = AliasBase::A; // uncheckable
  K = AliasBase::A; // uncheckable
}

// Inclusions on the aliasing enum disqualify the exemption: `L` aliases `A`,
// which is also brought in by `use` with value 1, so `L` and the inherited `A`
// actually collide. The no-inclusions gate keeps this from being wrongly
// exempted.
enum AliasWithUse: int as int {
  use AliasBase;
  L = AliasBase::A; // uncheckable
}
