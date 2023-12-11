<?hh


<<__EntryPoint>>
function main_1679() :mixed{
$dom = new DOMDocument();
$dom->loadXML('<a><b><c /></b></a>');
$remove = vec[];
foreach ($dom->getElementsByTagName('b') as $data) {
  foreach ($data->childNodes as $element) {
    if ($element is DOMElement) {
      $remove[] = $element;
    }
  }
}
foreach ($remove as $r) {
  $r->parentNode->removeChild($r);
}
echo $dom->saveXML();
}
