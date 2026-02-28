<?hh

enum class E: mixed {
  int A = 42;
}

function takes_label(HH\EnumClass\Label<E, int> $_): void {}

function call_it(): void {
  takes_label(#B);
}
