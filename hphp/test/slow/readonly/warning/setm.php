<?hh
<<file:__EnableUnstableFeatures('readonly')>>
class Bar {}
class Foo {
  public Bar $c;
  public readonly Bar $ro_c;
}

<<__EntryPoint>>
function test(): void {
  $t = new Foo();
  $t->c = new Bar();
  $t->ro_c = new Bar();
  $t->ro_c = readonly new Bar();
  $t->c = readonly new Bar();
}
