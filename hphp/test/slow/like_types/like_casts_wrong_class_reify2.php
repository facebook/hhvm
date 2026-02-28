<?hh
<<file:__EnableUnstableFeatures('like_type_hints')>>

class C<reify T> {}

<<__EntryPoint>>
function main(): void {
    $c = new C<int>();
    $c as ~C<string>;
}
