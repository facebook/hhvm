<?hh

class Foo {
  public int $bar = 0;
  public static int $baz = 0;
}

function f(): void {
  $member = "mbr";
  $a = dict[];
  $b = isset($a[0]); // okay.
  $b = isset($a[0][42]); // okay.
  $b = isset($a[0]->member); // nope.
  $b = isset(($a[0] as dynamic)->$member); // okay.
  $c = isset($b); // nope.
  $d = isset((new Foo())->bar); // nope.
  $bar = "bar";
  $dynfoo = new Foo() as dynamic;
  $e = isset($dynfoo->$bar); // okay.
  $baz = "baz";
  $g = isset(Foo::$baz); // okay.
  $h = isset($dynfoo::$baz); // okay?
  $i = isset($dynfoo::baz); // okay - treated as a class const.
}
