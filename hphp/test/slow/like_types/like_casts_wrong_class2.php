<?hh

class C<T> {}
class D {}

<<__EntryPoint>>
function main(): void {
    $d = new D();
    $d as ~D;
    $d as ~C<_>;
}
