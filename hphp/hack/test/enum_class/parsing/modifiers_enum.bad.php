<?hh

abstract enum E : int {
  X = 42;
}

enum F : int {
  abstract X;
}

final enum G : int {
  X = 42;
}

abstract final enum H : int {}

final abstract enum K : int {}
