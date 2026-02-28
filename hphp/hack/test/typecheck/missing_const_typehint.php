<?hh
class C {
  const X = 123; // This is OK; it's clear what the type is.
  const int Y = self::X; // This is OK.
  const Z = self::X; // This is not OK.
}
