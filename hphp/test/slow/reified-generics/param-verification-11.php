<?hh

type Ignore<T> = mixed;

class C<T> {}
class D {}

function f(Ignore<C<int>> $_) :mixed{}
<<__EntryPoint>> function main(): void {
f(new D());
echo "done\n";
}
