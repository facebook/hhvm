<?hh

function test($a, $b) {
  return array_map(function (varray $x) use ($b) {
      var_dump($x,$b);
    }
, $a);
}

<<__EntryPoint>>
function main_1937() {
test(varray[varray[1], varray[2]], 5);
}
