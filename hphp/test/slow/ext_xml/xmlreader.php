<?hh

function VS($x, $y) :mixed{
  var_dump($x === $y);
  if ($x !== $y) { echo "Failed: $y\n"; }
}



<<__EntryPoint>>
function main_xmlreader() :mixed{
$reader = new XMLReader();
$reader->XML("<?xml version=\"1.0\" encoding=\"UTF-8\"?><a y=\"\" z=\"1\"></a>");
$reader->read();
VS($reader->getAttribute("x"), null);
VS($reader->getAttribute("y"), "");
VS($reader->getAttribute("z"), "1");
}
