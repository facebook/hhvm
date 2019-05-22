<?php

function add_cssclass($add, $class) {
  $class = empty($class) ? $add : $class .= ' ' . $add;
  return $class;
}
<<__EntryPoint>> function main() {
add_cssclass('test', $a);
}
