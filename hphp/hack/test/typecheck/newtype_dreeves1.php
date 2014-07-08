//// newtype.php
<?hh // strict
newtype FooInt as int = int;
newtype FooString as string = string;
newtype FooArray<T> as array<T> = array<T>;
newtype FooMap<Tk, Tv> as Map<Tk, Tv> = Map<Tk, Tv>;

//// test.php
<?hh // strict

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

  $array[] = 1;
  $map[''] = '';
}
