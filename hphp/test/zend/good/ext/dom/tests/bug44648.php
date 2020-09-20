<?hh
<<__EntryPoint>> function main(): void {
$doc = new DOMDocument();
$doc->loadXML('<root/>');

$root = $doc->documentElement;

try {
  $attr = new DOMAttr('@acb', '123');
  $root->setAttributeNode($attr);
} catch (DOMException $e) {
  echo $e->getMessage()."\n";
}

try {
  $root->setAttribute('@def', '456');
} catch (DOMException $e) {
  echo $e->getMessage()."\n";
}

try {
  $root->setAttributeNS('', '@ghi', '789');
} catch (DOMException $e) {
  echo $e->getMessage()."\n";
}

try {
  $root->setAttributeNS('urn::test', 'a:g@hi', '789');
} catch (DOMException $e) {
  echo $e->getMessage()."\n";
}

echo $doc->saveXML($root);
}
