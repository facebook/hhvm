<?php

var_dump(class_exists('AppendIterator'));
var_dump(class_exists('IteratorIterator'));
var_dump(is_subclass_of('AppendIterator', 'IteratorIterator'));
$a = new AppendIterator;
var_dump($a instanceof IteratorIterator);
