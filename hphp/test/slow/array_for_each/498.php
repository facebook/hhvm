<?hh

function k4() :mixed{
  $arr = vec[0,1,2,3,4];
  $b = true;
  foreach ($arr as $v) {
    if ($b) {
      $b = false;
      $arr2 = $arr;
    }
    echo "val=$v\n";
  }
}

<<__EntryPoint>>
function main_498() :mixed{
k4();
}
