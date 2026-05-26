<?hh

// The class is the winning definition and the typedef is the losing definition.
// If the losing typedef were still observable as the source of truth for the
// type hint below, `$value + 1` would be accepted as arithmetic on `int`.
// The expected errors demonstrate that `ClassFirst` is instead treated as a class.
class ClassFirst {}
type ClassFirst = int;

function duplicate_class_typedef_bad_program(ClassFirst $value): int {
  return $value + 1;
}
