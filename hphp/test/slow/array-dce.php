<?hh

function foo(dict $d, arraykey $ak, int $i) {
  return $d[$ak][$i][$i] + 12;
}

function keyify(int $i) {
  return 'a'.$i;
}

function dictify(int $i) {
  $k = keyify($i);
  $r = dict[$k => dict[]];
  $r[$k][$i] = dict[];
  $r[$k][$i][$i] = $i * 10;
  return $r;
}

function cycle(int $i) {
  var_dump(foo(dictify($i), keyify($i), $i));
}

<<__EntryPoint>>
function main() {
  cycle(10); cycle(20); cycle(30); cycle(40);
}
