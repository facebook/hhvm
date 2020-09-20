<?hh

enum Foo: int {
  // ILlegal since Foo::class means something else.
  class = 0;
}
<<__EntryPoint>> function main(): void {
echo "Done\n";
}
