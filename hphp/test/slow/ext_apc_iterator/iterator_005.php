<?php
error_reporting(E_ALL & ~E_USER_NOTICE & ~E_NOTICE);

$vals = array();
$vals2 = array();
$it = new APCIterator('user', '/key[0-9]0/');
for($i = 0; $i < 41; $i++) {
  apc_store("key$i", "value$i");
}
apc_delete($it);
$it2 = new APCIterator('user');
foreach($it as $key=>$value) {
  $vals[$key] = $value['key'];
}
foreach($it2 as $key=>$value) {
  $vals2[$key] = $value['key'];
}
ksort($vals2);
var_dump($vals);
var_dump($vals2);

?>
===DONE===
<?php exit(0); ?>
