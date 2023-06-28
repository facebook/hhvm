<?hh

function f() :mixed{
  $x = Map {
'a' => 1, 'b' => 2, 'c' => 3, 'd' => 4}
;
  unset($x['a']);
  unset($x['c']);
  foreach ($x as $k => $v) {
    echo $k . ' ' . $v . "\n";
  }
}

<<__EntryPoint>>
function main_818() :mixed{
f();
}
