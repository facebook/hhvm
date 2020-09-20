<?hh <<__EntryPoint>> function main(): void {
$doc = new DOMDocument();
$doc->loadHTML("<html><body><p>Test<br></p></body></html>");
echo $doc->saveHTML();
}
