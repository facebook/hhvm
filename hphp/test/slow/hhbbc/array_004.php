<?hh

class obj1 { function heh() { echo "heh\n"; } }
class obj2 { function heh() { echo "yup\n"; } }
function stuff() {
  return varray[new obj1, new obj2];
}
function main() {
  list($x, $y) = stuff();
  $x->heh();
  $y->heh();
}

<<__EntryPoint>>
function main_array_004() {
main();
}
