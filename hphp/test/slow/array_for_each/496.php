<?hh

function k2() :mixed{
  $arr = vec[0,1,2,3,4];
  $arr2 = $arr;
  foreach ($arr as $v) {
    echo "val=$v\n";
  }
}

<<__EntryPoint>>
function main_496() :mixed{
k2();
}
