<?hh

// OptedOut gives two members the same value on purpose, and opts out of the
// duplicate-value check with <<__AllowUncheckedEnumValues>>. Its values are
// therefore not recorded (scc_value = None) -- contrast with the plain Normal
// enum below, whose values are recorded.
<<__AllowUncheckedEnumValues>>
enum OptedOut: int {
  A = 1;
  B = 1;
}

// A plain enum, with no opt-out attribute, for contrast.
enum Normal: int {
  C = 1;
  D = 2;
}
