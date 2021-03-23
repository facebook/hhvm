<?hh

enum B : int as int {
  X = B::X;
  Y = self::Y;
}


enum F : int as int {
  use E;
}

enum E : int as int {
  X = F::X;
}
