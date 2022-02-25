<?hh

abstract enum class E : mixed {
  abstract int X;
}

enum class H : mixed extends E {
  int X = 42;
}

abstract enum class G : mixed extends H {
  // hide concrete constant from parent
  abstract int X;
}
