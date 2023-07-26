<?hh

class C<T> {}

<<__EntryPoint>>
function main(): void {
    $c = new C();
    $c as ~C<int>;
}
