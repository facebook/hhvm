<?hh

function k1() :mixed{
  $arr = vec[0,1,2,3,4];
  foreach ($arr as $v) {
    echo "val=$v\n";
  }
}

<<__EntryPoint>>
function main_495() :mixed{
k1();
}
