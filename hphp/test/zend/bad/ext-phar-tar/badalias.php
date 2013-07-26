<?php
$e = dirname(__FILE__) . '/files/';
for ($i = 1; $i <= 5; $i++) {
try {
new Phar($e . "badalias$i.phar.tar");
} catch (Exception $ee) {
echo $ee->getMessage(), "\n";
}
}
?>
===DONE===