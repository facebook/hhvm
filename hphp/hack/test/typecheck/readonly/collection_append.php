<?hh
<<file: __EnableUnstableFeatures("readonly")>>
class Foo {
  public readonly static ?Foo $x = null;
  public function __construct(public int $prop = 4) {}
}
function test_cipp(readonly vec<Foo> $x, vec<Foo> $y, readonly shape('a' => Foo) $z) : void {
  $x[] = new Foo();
  $x[] = readonly new Foo();
  // error readonly assigned to mutable vec
  // todo, update the type of $y instead
  $y[] = readonly new Foo();
  $z['a'] = readonly new Foo();


  $d = dict[3 => readonly new Foo()];
  $d[5] = readonly new Foo(); // ok
  // is readonly
  $vector = Vector { readonly new Foo() };
  $vector[] = new Foo(); // error, readonly modified
}
