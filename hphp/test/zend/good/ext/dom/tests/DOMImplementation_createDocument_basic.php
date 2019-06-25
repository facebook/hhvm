<?hh <<__EntryPoint>> function main(): void {
$x = new DOMImplementation();
$doc = $x->createDocument(null, 'html');
echo $doc->saveHTML();
}
