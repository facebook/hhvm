<?hh

function foo(dict $d, arraykey $ak, int $i) :mixed{
  return $d[$ak][$i][$i] + 12;
}

function keyify(int $i) :mixed{
  return 'a'.$i;
}

function dictify(int $i) :mixed{
  $k = keyify($i);
  $r = dict[$k => dict[]];
  $r[$k][$i] = dict[];
  $r[$k][$i][$i] = $i * 10;
  return $r;
}

function cycle(int $i) :mixed{
  var_dump(foo(dictify($i), keyify($i), $i));
}

<<__EntryPoint>>
function main() :mixed{
  cycle(10); cycle(20); cycle(30); cycle(40);
}
