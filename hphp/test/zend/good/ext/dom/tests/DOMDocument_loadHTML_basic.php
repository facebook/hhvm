<?hh <<__EntryPoint>> function main() {
$doc = new DOMDocument();
$doc->loadHTML("<html><body><p>Test<br></p></body></html>");
echo $doc->saveHTML();
}
