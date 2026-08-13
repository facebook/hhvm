<?hh

// An aliased member inherited from an opted-out enum is unverified, so it counts
// as uncheckable and the alias is not exempted. This keeps
// `<<__AllowUncheckedEnumValues>>` downward-closed across the alias edge.

<<__AllowUncheckedEnumValues>>
enum OptedOutBase: int as int {
  OA = 1;
  OB = 2;
}

enum UsesOptedOut: int as int {
  use OptedOutBase; // OA = 1, OB = 2 -- unverified because OptedOutBase opted out
  UO = 3;
}

enum AliasOfUsesOptedOut: int as int {
  T = UsesOptedOut::OA; // uncheckable (OA originates in an opted-out enum)
  U = UsesOptedOut::UO;
}
