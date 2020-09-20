<?hh
<<__EntryPoint>> function main(): void {
$xml = new SimpleXmlElement("<img></img>");
$xml->addAttribute("src", "foo");
$xml->addAttribute("alt", "");
echo $xml->asXML();

echo "===DONE===\n";
}
