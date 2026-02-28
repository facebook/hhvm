<?hh
<<file:__EnableUnstableFeatures('like_type_hints')>>

class C<T> {}

<<__EntryPoint>>
function main(): void {
    $c = new C();
    $c as ~C<int>;
}
