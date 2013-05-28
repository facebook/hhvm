<?php

$dom = new DOMDocument();
$dom->loadXML('<a><b><c /></b></a>');
$remove = array();
foreach ($dom->getElementsByTagName('b') as $data) {
  foreach ($data->childNodes as $element) {
    if ($element instanceof DOMElement) {
      $remove[] = $element;
    }
  }
}
foreach ($remove as $r) {
  $r->parentNode->removeChild($r);
}
echo $dom->saveXML();
