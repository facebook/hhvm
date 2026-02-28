<?hh

function main($arr1, $arr2) :mixed{
  $tot = 0;
  foreach($arr1 as $v1) {
    foreach($arr2 as $v2) {
      $tot += $v1 * $v2;
    }
  }
  return $tot;
}


<<__EntryPoint>>
function main_nested_foreach1() :mixed{
$a = vec[1,2,3];
$b = vec[10,20,30];

for ($i = 0; $i < 10; $i++) {
  var_dump(main($a, $b));
}
}
