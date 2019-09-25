<?hh

function main() {
  $v = vec[1, 2, 3];
  var_dump($v);
  uksort(inout $v, ($p1, $p2) ==> {
      return $p1 <=> $p2;
    });
  var_dump($v);
  return $v;
}


<<__EntryPoint>>
function main_uksort_vec() {
var_dump(main());
}
