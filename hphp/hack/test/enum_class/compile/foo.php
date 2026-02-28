<?hh

enum class A : mixed {
  int LA = 42;
}

enum class B : mixed extends A {
  int LB = 43;
}
