<?hh
<<__EntryPoint>> function main(): void {
$doc = new DOMDocument;

$doc->load(dirname(__FILE__)."/book.xml");

$result = $doc->schemaValidate('');
var_dump($result);
}
