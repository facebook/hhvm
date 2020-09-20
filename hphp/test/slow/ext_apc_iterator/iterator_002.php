<?hh <<__EntryPoint>> function main(): void {
error_reporting(E_ALL & ~E_USER_NOTICE & ~E_NOTICE);

$it = new APCIterator('user', '/key[0-9]0/');
for($i = 0; $i < 41; $i++) {
  apc_store("key$i", "value$i");
}
$vals = darray[];
foreach($it as $key=>$value) {
  $vals[$key] = $value['key'];
}
ksort(inout $vals);
var_dump($vals);

echo "===DONE===\n";
}
