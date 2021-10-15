<?hh

// we don't allow class as a name, which would make ::class ambiguous
enum class E : int {
  int class = 42;
}
