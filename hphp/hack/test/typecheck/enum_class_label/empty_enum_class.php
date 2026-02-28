<?hh

enum class E: mixed {
}

function get(HH\EnumClass\Label<E, int> $label): void {}

function test(): void {
  get(#X);
  E#X;
}
