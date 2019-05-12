<?hh

class C {}

// Dont check the generics if the class does not have
// any reified generics
function f(C<D> $x) {}
<<__EntryPoint>> function main(): void {
f(new C());
echo "ok\n";
}
