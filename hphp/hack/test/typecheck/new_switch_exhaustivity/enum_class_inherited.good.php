<?hh

enum class E: mixed {
  int IntE = 0;
  string StringE = "";
}

enum class F: mixed extends E {
  int IntF = 0;
  string StringF = "";
}

function test_inherited_enum_class1(HH\EnumClass\Label<F, mixed> $f): void {
  switch ($f) {
    case F#IntF:
      return;
    case F#StringF:
      return;
    case E#IntE:
      return;
    case E#StringE:
      return;
  }
}

function test_inherited_enum_class2(HH\EnumClass\Label<F, mixed> $f): void {
  switch ($f) {
    case F#IntF:
      return;
    case F#StringF:
      return;
    case F#IntE:
      return;
    case F#StringE:
      return;
  }
}

function test_inherited_enum_class3(HH\EnumClass\Label<F, string> $f): void {
  switch ($f) {
    case F#StringF:
      return;
    case E#StringE:
      return;
  }
}

function test_inherited_enum_class4(HH\EnumClass\Label<F, string> $f): void {
  switch ($f) {
    case #StringF:
      return;
    case #StringE:
      return;
  }
}
