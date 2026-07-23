<?hh
<<file:__EnableUnstableFeatures('case_types')>>

enum IntEnum : int as int {}
enum StrEnum : string as string {}

// Enums are treated as an ambiguous subset of arraykey regardless of their
// declared base type, so these mixed variants must be rejected.
case type BadIntEnumString = IntEnum | string;
case type BadStrEnumInt = StrEnum | int;

// A single enum on its own with a disjoint tag is still fine.
case type OkEnumFloat = IntEnum | float;
case type OkEnumNull = StrEnum | null;
