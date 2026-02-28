<?hh

class obj1 { function heh() :mixed{ echo "heh\n"; } }
class obj2 { function heh() :mixed{ echo "yup\n"; } }
function stuff() :mixed{
  return vec[new obj1, new obj2];
}
function main() :mixed{
  list($x, $y) = stuff();
  $x->heh();
  $y->heh();
}

<<__EntryPoint>>
function main_array_004() :mixed{
main();
}
