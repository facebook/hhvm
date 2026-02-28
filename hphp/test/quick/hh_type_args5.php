<?hh
interface I {}
interface J {}
interface K<T> {}
// Three supertypes
class A<T as I as J as K<int>> {}
// A supertype and a subtype
class B<T as I, Tu super T as J> {
  // And in a method
  public function foo<Tf as T as I>(Tf $x) :mixed{}
  public function bar<Tb super T as K<string>>(Tb $y) :mixed{}
}
// Same with functions
function bar<T1, Tu super T1 as B<T1,Tu>>(T1 $x, Tu $y) :mixed{}
<<__EntryPoint>> function main(): void {
echo "Done\n";
}
