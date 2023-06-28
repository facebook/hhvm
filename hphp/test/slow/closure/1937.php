<?hh

function test($a, $b) :mixed{
  return array_map(function (varray $x) use ($b) {
      var_dump($x,$b);
    }
, $a);
}

<<__EntryPoint>>
function main_1937() :mixed{
test(varray[varray[1], varray[2]], 5);
}
