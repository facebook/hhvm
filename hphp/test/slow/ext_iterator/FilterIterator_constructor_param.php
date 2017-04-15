<?php

$iterators = array('FilterIterator', 'RegexIterator', 'RecursiveFilterIterator');
foreach ($iterators as $iterator) {
  $class = new ReflectionClass($iterator); 
  $params = (array)$class->getConstructor()->getParameters();
  var_dump(count($params) && $params[0]->getName() == 'iterator');
}
