<?php
function __autoload($class)
{
  if (!array_key_exists('include', $GLOBALS)) $GLOBALS['include'] = array();
  $GLOBALS['include'][] = $class;
  eval("class DefClass{}");
}

$a = new DefClass;
print_r($a);
print_r($GLOBALS['include']);
