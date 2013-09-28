<?php

function VS($x, $y) {
  var_dump($x === $y);
  if ($x !== $y) { echo "Failed: $y\n"; }
}


$reader = new XMLReader();
$reader->xml("<?xml version=\"1.0\" encoding=\"UTF-8\"?><a y=\"\" z=\"1\"></a>");
$reader->read();
VS($reader->getattribute("x"), null);
VS($reader->getattribute("y"), "");
VS($reader->getattribute("z"), "1");
