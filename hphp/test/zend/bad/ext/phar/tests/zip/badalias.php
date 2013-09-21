<?php
$e = dirname(__FILE__) . '/files/';
for ($i = 1; $i <= 5; $i++) {
try {
new Phar($e . "badalias$i.phar.zip");
} catch (Exception $ee) {
echo $ee->getMessage(), "\n";
}
}
?>
===DONE===