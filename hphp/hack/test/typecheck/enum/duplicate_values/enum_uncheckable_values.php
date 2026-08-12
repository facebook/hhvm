<?hh

// With check_duplicate_enum_values on (see HH_FLAGS), an enum member whose value
// cannot be statically evaluated (constant reference, arithmetic, ...) is flagged
// as uncheckable: the enum can't be verified free of duplicate values, so it
// needs a literal value or the opt-out attribute.

class Bar {
  const int C = 1;
}

// 1. Constant reference: value can't be statically checked.
enum EnumConstRef: int as int {
  A = Bar::C; // uncheckable
  B = 2;
}

// 2. Arithmetic: computed value can't be statically checked.
enum EnumArith: int as int {
  X = 1 + 1; // uncheckable
  Y = 3;
}

// 3. Several uncheckable members are reported together, on the enum.
enum EnumManyComputed: int as int {
  P = Bar::C; // uncheckable
  Q = 1 + 1; // uncheckable
}

// 4. No false positive: an all-literal enum is fully checkable, no error.
enum EnumAllLiteral: int as int {
  M = 1;
  N = 2;
}

// 5. Enum classes use a different declaration form and are not checked at all,
//    even when two members share a value.
enum class EnumClassNotChecked: int {
  int A = 1;
  int B = 1;
}
