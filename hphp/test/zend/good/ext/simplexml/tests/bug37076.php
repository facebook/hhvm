<?hh <<__EntryPoint>> function main(): void {
$xml = simplexml_load_string("<root><foo /></root>");
$xml->foo = "foo";
$xml->foo = $xml->foo->__toString() . "bar";
print $xml->asXML();
echo "===DONE===\n";
}
