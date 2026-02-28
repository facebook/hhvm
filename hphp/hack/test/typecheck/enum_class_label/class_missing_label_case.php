<?hh

enum class E: mixed {
  int car = 1;
  int CARROT = 42;
}

function takes_label(HH\EnumClass\Label<E, int> $_): void {}

function call_it(): void {
  takes_label(#carrot);
}
