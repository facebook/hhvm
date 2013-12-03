<?php

//file
mkdir('test_dir_ptfi');
$dirIterator = new DirectoryIterator('test_dir_ptfi');
var_dump(decoct($dirIterator->getInode()));

?>
<?php
rmdir('test_dir_ptfi');
?>