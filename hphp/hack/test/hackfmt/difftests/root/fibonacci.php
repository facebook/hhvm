<?hh // strict

function fibonacci($pos){
  $arr = varray[0, 1];
  for ($i = 2; $i <= $pos; ++$i)
    $arr[$i] =
      $arr[$i-1] + $arr[$i-2];
  return $arr[$pos];
}
