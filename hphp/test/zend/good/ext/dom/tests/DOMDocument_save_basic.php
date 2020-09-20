<?hh <<__EntryPoint>> function main(): void {
$doc = new DOMDocument('1.0');
$doc->formatOutput = true;

$root = $doc->createElement('book');

$root = $doc->appendChild($root);

$title = $doc->createElement('title');
$title = $root->appendChild($title);

$text = $doc->createTextNode('This is the title');
$text = $title->appendChild($text);

$temp_filename = __SystemLib\hphp_test_tmppath('DomDocument_save_basic.tmp');

echo 'Wrote: ' . $doc->save($temp_filename) . ' bytes'; // Wrote: 72 bytes

unlink($temp_filename);
}
