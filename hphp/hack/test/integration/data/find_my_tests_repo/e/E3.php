<?hh

enum class E3_Super: int {
  int VALUE1 = 1;
}

enum class E3_Sub: int extends E3_Super {
  int VALUE2 = 2;
}
