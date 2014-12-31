<?php

// Characters we know not to be numbers.
$characters = array(' ', '#', 'a', '$', '_');

// Numbers (hex and dec) we know to be valid HTML entities.
foreach (range(165, 170) as $n) {
  array_push($characters, $n);
  array_push($characters, 'x' . dechex($n));
}

foreach ($characters as $c) {
  var_dump(html_entity_decode(sprintf('&#%s;', $c)));
}
