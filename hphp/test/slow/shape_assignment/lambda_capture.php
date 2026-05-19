<?hh

<<file:__EnableUnstableFeatures('shape_and_tuple_destructuring')>>;

<<__EntryPoint>>
function main(): void {
  $s = shape('x' => 'a', 'y' => 'b');
  shape('x' => $x, 'y' => $y) = $s;
  $f = () ==> $x.$y;
  echo "f()="; var_dump($f());

  $t = tuple('hello', ' world');
  tuple($a, $b) = $t;
  $g = () ==> $a.$b;
  echo "g()="; var_dump($g());

  shape('x' => $x2, ?'missing' => $m, ...) = $s;
  $h = () ==> tuple($x2, $m);
  echo "h()="; var_dump($h());
}
