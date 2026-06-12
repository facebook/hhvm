<?hh

// Duplicate enum member values should be rejected by the typechecker.
// Today this typechecks clean and only blows up at runtime; these cases should
// start producing an error once the duplicate-value check is added.

// 1. Direct duplicate: two members with the same value.
enum EnumDupValues: int as int {
  Low = 1;
  Medium = 1; // duplicate value of Low
  High = 3;
}

// 2. Duplicate introduced by enum inclusion: an included member and a locally
//    declared member share a value.
enum EnumDupBase: int as int {
  A = 1;
  B = 2;
}

enum EnumDupViaUse: int as int {
  use EnumDupBase;
  C = 1; // collides with EnumDupBase::A (= 1)
}

// 3. Duplicate from including two enums whose values overlap. Each base is fine
//    on its own; the collision only exists in the enum that includes both.
enum EnumDupIncludedOne: int as int {
  X = 10;
}

enum EnumDupIncludedTwo: int as int {
  Y = 10; // same value as EnumDupIncludedOne::X
}

enum EnumDupIncludesBoth: int as int {
  use EnumDupIncludedOne;
  use EnumDupIncludedTwo;
}
