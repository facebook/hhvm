<?hh
type T = string;

class Foo {}

class Baz {}

class Bar {
  public string $p1 = Baz::class;
  public T $p2 = Baz::class;
  public @HH\classname $p3 = Baz::class;
  public arraykey $p4 = Baz::class;
  public static string $sp1 = Baz::class;
  public static T $sp2 = Baz::class;
  public static @HH\classname $sp3 = Baz::class;
  public static arraykey $sp4 = Baz::class;
  public string $lp1 = Baz::class;
  public T $lp2 = Baz::class;
  public @HH\classname $lp3 = Baz::class;
  public arraykey $lp4 = Baz::class;
  public static string $slp1 = Baz::class;
  public static T $slp2 = Baz::class;
  public static @HH\classname $slp3 = Baz::class;
  public static arraykey $slp4 = Baz::class;
}

function foo1(string $x) : string {
  var_dump($x);
  return Foo::class;
}

function foo2(@HH\classname $x) : @HH\classname {
  var_dump($x);
  return Foo::class;
}

function foo3(T $x) : T {
  var_dump($x);
  return Foo::class;
}

function foo4(inout @HH\classname $x) :mixed{
  var_dump($x);
  $x = Baz::class;
}

function foo5(arraykey $x) : arraykey {
  var_dump($x);
  return Foo::class;
}

function foo6<reify T>(T $x) : T {
  var_dump($x);
  return Foo::class;
}

function foo7(@HH\classname $a, inout arraykey $b) : string {
  var_dump($a);
  var_dump($b);
  $b = __hhvm_intrinsics\launder_value(Baz::class);
  return __hhvm_intrinsics\launder_value(Foo::class);
}

<<__EntryPoint>>
function main() :mixed{
  var_dump(foo1(Foo::class));
  var_dump(foo2(Foo::class));
  var_dump(foo3(Foo::class));
  $c = Foo::class;
  foo4(inout $c);
  var_dump($c);
  var_dump(foo5(Foo::class));
  var_dump(foo6<HH\classname>(Foo::class)); // TODO(T174926790) I expected this to fail, reified generics bug
  var_dump(foo6<arraykey>(Foo::class));
  $d = __hhvm_intrinsics\launder_value(Foo::class);
  var_dump(foo7(__hhvm_intrinsics\launder_value(Foo::class), inout $d));
  var_dump($d);
  $c = Bar::class;
  $o = new $c;
  var_dump($o->p1);
  $o->p1 = __hhvm_intrinsics\launder_value(Foo::class);
  var_dump($o->p1);
  var_dump($o->p2);
  $o->p2 = Foo::class;
  var_dump($o->p2);
  var_dump($o->p3);
  $o->p3 = Foo::class;
  var_dump($o->p3);
  var_dump($o->p4);
  $o->p4 = Foo::class;
  var_dump($o->p4);
  var_dump($c::$sp1);
  $c::$sp1 = Foo::class;
  var_dump($c::$sp1);
  var_dump($c::$sp2);
  $c::$sp2 = Foo::class;
  var_dump($c::$sp2);
  var_dump($c::$sp3);
  $c::$sp3 = Foo::class;
  var_dump($c::$sp3);
  var_dump($c::$sp4);
  $c::$sp3 = Foo::class;
  var_dump($c::$sp4);
}
