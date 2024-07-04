<?hh

enum class E: mixed {
  const type T = int;
}

function get(HH\EnumClass\Label<E, int> $label): void {}

function test(): void {
  get(#T);
  E#T;
}
