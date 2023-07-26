<?hh

class C<reify T> {}

<<__EntryPoint>>
function main(): void {
    $c = new C<int>();
    $c as ~C<string>;
}
