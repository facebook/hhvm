<?hh

enum class E : int {
  int A = 42;
}

function main(): void {
  E::valueOf#A(); // experimental feature
}
