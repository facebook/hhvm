<?hh

class Foo {
}

class Bar {
}

class Baz {
  public static dict<HH\classname, int> $dd = dict[Foo::class => 10];
  public static Map<mixed, int> $mm = Map{Foo::class => 10};
  public static mixed $c = Foo::class;
  public static Vector<HH\classname> $vv = Vector {};

  public static function dict_fun() {
    echo '===dict:===';
    self::$dd[static::class] = 42;
    var_dump(isset(self::$dd[static::class]));
    var_dump(self::$dd[static::class]);
    var_dump(array_key_exists(static::class, self::$dd));
    var_dump(idx(self::$dd, Foo::class));
  }

  public static function Map_fun() {
    echo '===Map:===';
    self::$mm[self::$c] = 42;
    var_dump(isset(self::$mm[static::class]));
    var_dump(self::$mm[self::$c]);
    var_dump(array_key_exists(static::class, self::$mm));
    var_dump(idx(self::$mm, self::$c));
  }

  public static function Vector_fun() {
    echo '===Vector:===';
    self::$vv[] = Baz::class;
    self::$vv[] = Fizz::class; // not a class
    self::$vv[] = self::$c;
    var_dump(self::$vv);
  }
}

function keyset_fun() {
    echo '===keyset:===';
    $a = keyset[Foo::class];
    $a[] = Bar::class;
    var_dump($a);
    unset($a[Bar::class]);
    var_dump(array_key_exists(Bar::class, $a));
    var_dump($a[Foo::class]);
}

function Set_fun() {
  echo '===Set===';
  $s = Set {Foo::class};
  $s[] = Bar::class;
  var_dump($s);
  unset($s[Bar::class]);
  var_dump(array_key_exists(Bar::class, $s));
  var_dump($s[Foo::class]);
  var_dump(idx($s, Foo::class));
}

function vec_fun() {
  echo '===vec:===';
  $a = vec[Foo::class, Fizz::class]; // Fizz is not a class
  $a[] = Baz::class;
  var_dump($a);
  foreach ($a as $v) {
    var_dump($v);
  }
}

function dvarray_fun() {
  echo '===dvarray:===';
  $a = varray[];
  $a[] = Baz::class;
  $a[] = Fizz::class;
  var_dump($a);
  var_dump(array_keys($a));
  $b = darray[Foo::class => Baz::class];
  $b[Baz::class] = Fizz::class;
  var_dump(array_keys($b));
}

function shape_fun() {
  echo '===shape===';
  $x = shape(Foo::class => 1);
  $x[Bar::class] = 2;
  $x['Foo'] = 3;
  $x[Fizz::class] = 4;
  var_dump($x);
  var_dump(idx($x, Foo::class));
}

<<__EntryPoint>>
function main() {
  keyset_fun();
  Baz::dict_fun();
  Set_fun();
  Baz::Map_fun();
  vec_fun();
  Baz::Vector_fun();
  dvarray_fun();
  shape_fun();
}
