<?hh

enum class E: mixed {
  int Int1 = 0;
  int Int2 = 1;
  string String1 = "apple";
}

function test_enum_class1(HH\MemberOf<E, int> $e): void {
  switch ($e) {
    case E::Int1:
      return;
    case E::Int2:
      return;
  }
}

function test_enum_class2(HH\MemberOf<E, string> $e): void {
  switch ($e) {
    case E::String1:
      return;
  }
}

function test_enum_class3<T>(HH\MemberOf<E, T> $e): void {
  switch ($e) {
    case E::Int1:
      return;
    case E::Int2:
      return;
    case E::String1:
      return;
  }
}

function test_enum_class_label1(HH\EnumClass\Label<E, int> $e): void {
  switch ($e) {
    case E#Int1:
      return;
    case E#Int2:
      return;
  }
}

function test_enum_class_label2(HH\EnumClass\Label<E, string> $e): void {
  switch ($e) {
    case E#String1:
      return;
  }
}

function test_enum_class_label3<T>(HH\EnumClass\Label<E, T> $e): void {
  switch ($e) {
    case E#Int1:
      return;
    case E#Int2:
      return;
    case E#String1:
      return;
  }
}

function test_enum_class_short_label1(HH\EnumClass\Label<E, int> $e): void {
  switch ($e) {
    case #Int1:
      return;
    case #Int2:
      return;
  }
}

function test_enum_class_short_label2(HH\EnumClass\Label<E, string> $e): void {
  switch ($e) {
    case #String1:
      return;
  }
}

function test_enum_class_short_label3<T>(HH\EnumClass\Label<E, T> $e): void {
  switch ($e) {
    case #Int1:
      return;
    case #Int2:
      return;
    case #String1:
      return;
  }
}
