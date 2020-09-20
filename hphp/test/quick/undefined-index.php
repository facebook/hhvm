<?hh
<<__EntryPoint>> function main(): void {
$arr = varray[];
try {
  $a = $arr[123];
  echo $a;
} catch (Exception $e) { echo $e->getMessage()."\n"; }

$obj = new stdclass;
$a = $obj->flubb;
echo $a;
}
