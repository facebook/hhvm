<?php
$category = array("f");
$tree = array($category);

$iterator = new RecursiveIteratorIterator(
    new RecursiveArrayIterator($tree),
    RecursiveIteratorIterator::SELF_FIRST
);
foreach($iterator as $file);
echo "ok\n";
