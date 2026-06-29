<?hh

// OptedOut gives two members the same value on purpose. The diff that adds the
// <<__AllowDuplicateValues>> attribute opts it out of the duplicate-value check;
// contrast its recorded member values (scc_value) with the plain Normal enum.
enum OptedOut: int {
  A = 1;
  B = 1;
}

// A plain enum, with no opt-out attribute, for contrast.
enum Normal: int {
  C = 1;
  D = 2;
}
