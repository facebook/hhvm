<?hh
<<__EntryPoint>> function main(): void {
$arr = vec[1,2,3];
var_dump(apc_store('bluh', $arr));

$bla = __hhvm_intrinsics\apc_fetch_no_check('bluh');
foreach ($bla as $idx => $num) {
  $bla[$idx]++;
}
var_dump($bla);
var_dump($arr);
}
