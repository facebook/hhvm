<?hh
<<file:__EnableUnstableFeatures('like_type_hints')>>

class C<T> {}
class D {}

<<__EntryPoint>>
function main(): void {
    $d = new D();
    $d as ~D;
    $d as ~C<_>;
}
