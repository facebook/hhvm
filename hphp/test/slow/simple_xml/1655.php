<?php

function printElement($el, $indent='') {
  if (strlen($indent) > 10) {
    var_dump('Recursed to deep, backing out');
    return;
  }
  print $indent.$el->getName()."\n";
  foreach ($el->attributes() as $k => $v) {
    print $indent.$k.' => '.$v."\n";
  }
  foreach ($el->children() as $child) {
    printElement($child, $indent.'  ');
  }
}
$a = simplexml_load_string('<?xml version="1.0" encoding="utf-8"?><xx><yy><node a="b">hi</node></yy><yy><node a="b">hi</node></yy></xx>');
printElement($a);
