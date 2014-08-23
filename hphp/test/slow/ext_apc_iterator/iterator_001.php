<?php
error_reporting(E_ALL & ~E_USER_NOTICE & ~E_NOTICE);

$it = new APCIterator('user');
for($i = 0; $i < 41; $i++) {
  apc_store("key$i", "value$i");
}
foreach($it as $key=>$value) {
  $vals[$key] = $value['key'];
}
ksort($vals);
var_dump($vals);

?>
===DONE===
<?php exit(0); ?>
