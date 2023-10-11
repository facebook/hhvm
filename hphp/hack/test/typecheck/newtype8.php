//// newtype.php
<?hh
newtype FooInt as int = int;
newtype FooString as string = string;
newtype FooArray<T> as varray<T> = varray<T>;
newtype FooMap<Tk as arraykey, Tv> as Map<Tk, Tv> = Map<Tk, Tv>;

//// test.php
<?hh

function test_ops_with_newtypes(
  FooInt $x,
  FooString $y,
  FooArray<mixed> $array,
  FooMap<string, mixed> $map,
): void {
  $b = $y < '';
  hh_show($b);

  $s = "$x";
  hh_show($s); // string not FooString

  $t = $y . $x;
  hh_show($t); // string not FooString

  $c = $x << 1;
  hh_show($c); // int not FooInt

  $p = $x % 100;
  hh_show($p); // int not FooInt

  $array[] = 1;
  $map[''] = '';
}
