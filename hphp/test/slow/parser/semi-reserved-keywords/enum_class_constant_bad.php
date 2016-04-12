<?hh

enum Foo: int {
  // ILlegal since Foo::class means something else.
  class = 0;
}
echo "Done\n";
