<?hh

function main($a, $b, $c, $d, $e, $f) :mixed{
  $arr = vec[1,2,3,4,5,6,7,8];
  var_dump($a, $b, $c, $d, $arr[2]);
}
<<__EntryPoint>> function main_entry(): void {
main(1,2,3,4,5,6);
}
