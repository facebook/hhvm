<?hh
<<__EntryPoint>> function main(): void {
$filename = __SystemLib\hphp_test_tmppath('formatOutput.html');
$doc = new DOMDocument('1.0');
$doc->formatOutput = true;
$root = $doc->createElement('html');
$root = $doc->appendChild($root);
$head = $doc->createElement('head');
$head = $root->appendChild($head);
$title = $doc->createElement('title');
$title = $head->appendChild($title);
$text = $doc->createTextNode('This is the title');
$text = $title->appendChild($text);
$bytes = $doc->saveHTMLFile($filename);
var_dump($bytes);
echo file_get_contents($filename);
unlink($filename);
}
