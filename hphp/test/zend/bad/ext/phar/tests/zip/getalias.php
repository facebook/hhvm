<?php

$fname = dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.zip';

// sanity check with a virgin phar.zip
$phar = new Phar($fname);
var_dump($phar->getAlias());
unset($phar);

copy(dirname(__FILE__) . '/files/metadata.phar.zip', $fname);

// existing phar.zip, no alias set
$phar = new Phar($fname);
var_dump($phar->getAlias());

// check that default alias can be overwritten
$phar->setAlias('jiminycricket');
var_dump($phar->getAlias());
unset($phar);

// existing phar.zip, alias set
$phar = new Phar($fname);
var_dump($phar->getAlias());

// check that alias can't be set manually
try {
    $phar['.phar/alias.txt'] = 'pinocchio';
} catch (Exception $e) {
    echo $e->getMessage()."\n";
}
var_dump($phar->getAlias());

// check that user-defined alias can be overwritten
$phar->setAlias('pinocchio');
var_dump($phar->getAlias());

?>
===DONE===
<?php 
unlink(dirname(__FILE__) . '/' . basename(__FILE__, '.php') . '.phar.zip');
__HALT_COMPILER();
?>