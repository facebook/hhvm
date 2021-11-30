<?hh

enum class E : mixed {
  int __YOLO = 42;
  int X = 42;
}

enum F : int {
  __YOLO = 42;
}

function show(HH\EnumClass\Label<E, int> $label): void {
  echo E::valueOf($label);
  echo "\n";
}

<<__EntryPoint>>
function f(): void {
  echo E::__YOLO;
  echo "\n";
  echo F::__YOLO;
  echo "\n";
  show(#X);
  show(#__YOLO);
}
