<?hh


<<__EntryPoint>>
function main_228() :mixed{
$foo = vec[1,2,3,4];
foreach ($foo as $key => $val) {
  if($val == 2) {
    $foo[$key] = 0;
  }
 else if($val == 4) {
    unset($foo[$key]);
  }
 else {
    $foo[$key] = $val + 1;
  }
}
var_dump($foo);
}
