<?php

$arr = new RecursiveArrayIterator([['test2', 'test3']]);
$regex = new RecursiveRegexIterator($arr, '/^test/',
           RecursiveRegexIterator::ALL_MATCHES, 1, 2);

$child = $regex->getChildren();
var_dump(get_class($child),
         $child->getMode(),
         $child->getFlags(),
         $child->getPregFlags()
         );
