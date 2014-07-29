<?php
$phar = __DIR__ . '/files/stuboflength1041.phar';
foreach (new RecursiveIteratorIterator(new Phar($phar, null, 'stuboflength1041.phar')) as $item) {
    var_dump($item->getFileName());
}
?>
===DONE===