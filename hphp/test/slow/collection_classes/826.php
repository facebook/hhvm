<?hh

function f() {
  $vec = Vector {
1, 'b'}
;
  var_dump($vec->containsKey(0));
  var_dump($vec->containsKey(1));
  var_dump($vec->containsKey(2));
  echo "------------------------\n";
  $mp = Map {
'a' => 1, 2 => 'b'}
;
  var_dump($mp->containsKey('a'));
  var_dump($mp->containsKey(2));
  var_dump($mp->containsKey('b'));
  echo "------------------------\n";
  $smp = StableMap {
'a' => 1, 2 => 'b'}
;
  var_dump($smp->containsKey('a'));
  var_dump($smp->containsKey(2));
  var_dump($smp->containsKey('b'));
  echo "------------------------\n";
  $pair = Pair {
1, 'b'}
;
  var_dump($pair->containsKey(0));
  var_dump($pair->containsKey(1));
  var_dump($pair->containsKey(2));
}
f();
