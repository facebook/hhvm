<?hh
<<__SupportDynamicType>>
class Foo {
  public int $prop = 44;
}
<<__EntryPoint>>
async function foo(): Awaitable<void> {

  $x = readonly new Foo();
  $y = HH\FIXME\UNSAFE_CAST<Foo, Foo>($x);
  $y->prop = 5;
}
