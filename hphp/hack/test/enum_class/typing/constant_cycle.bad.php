<?hh

enum class B : int {
  int X = B::X;
  int Y = self::Y;
}


enum class F : int extends E {
}

enum class E : int {
  int X = F::X;
}
