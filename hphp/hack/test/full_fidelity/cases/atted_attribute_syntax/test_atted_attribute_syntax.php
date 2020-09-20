<?hh // strict

@Attr1
@Attr2
class C<T>{
  @__Soft protected int $x;
  <<__Soft, Attr3>> protected int $y;

  @__Rx
  public function f(): @__Soft void {}
}

@__Rx @__MutableReturn
async function f(): @__Soft Awaitable<@__Soft int> {
  return 42;
}
