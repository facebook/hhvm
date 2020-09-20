<?hh
class It { public $n = 0; }
<<__EntryPoint>> function main(): void {
$a = 1;
$it = new It;
$x = function ($x) use ($a, $it) {
  $it->n++;
  $a = $it->n.':'.$a;
  echo $x.':'.$a."\n";
};
$x(1);
$x(2);
$x(3);
}
