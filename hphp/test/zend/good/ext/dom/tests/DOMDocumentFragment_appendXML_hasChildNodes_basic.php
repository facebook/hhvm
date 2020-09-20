<?hh <<__EntryPoint>> function main(): void {
$doc = new DOMDocument();

$fragment = $doc->createDocumentFragment();
if ($fragment->hasChildNodes()) {
  echo "has child nodes\n";
} else {
  echo "has no child nodes\n";
}
$fragment->appendXML('<foo>bar</foo>');
if ($fragment->hasChildNodes()) {
  echo "has child nodes\n";
} else {
  echo "has no child nodes\n";
}
}
