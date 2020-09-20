<?hh

class C<reify T> {}

function c_int(): C<int> {
  return new C<~int>();
}

function vector_c_int(): Vector<C<int>> {
  return Vector<C<~int>> {};
}
