<?hh

enum class YOLO : mixed {
  int POUET = 42;
}

function f(HH\EnumClass\Label<YOLO, int> $label, int $x) : void {}

function main(): void {
  f(#AUTO332
}
