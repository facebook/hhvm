<?hh

enum class E : int {
  int A = 42;
  int B = 1664;
}

function main(): void {
  $x = E#A#B;
}
