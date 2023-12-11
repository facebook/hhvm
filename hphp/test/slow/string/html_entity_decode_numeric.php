<?hh


// Characters we know not to be numbers.
<<__EntryPoint>>
function main_html_entity_decode_numeric() :mixed{
$characters = vec[' ', '#', 'a', '$', '_'];

// Numbers (hex and dec) we know to be valid HTML entities.
foreach (range(165, 170) as $n) {
  array_push(inout $characters, $n);
  array_push(inout $characters, 'x' . dechex($n));
}

foreach ($characters as $c) {
  var_dump(html_entity_decode(sprintf('&#%s;', $c)));
}
}
