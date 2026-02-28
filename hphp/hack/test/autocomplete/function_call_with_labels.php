<?hh

enum class MyEnum: mixed {
  bool a_bool = true;
}

function label_taker(
  HH\EnumClass\Label<MyEnum, bool> $label1,
  int $i,
  HH\EnumClass\Label<MyEnum, bool> $label2
): void {}

function call_it(): void {
  label_taAUTO332
}
