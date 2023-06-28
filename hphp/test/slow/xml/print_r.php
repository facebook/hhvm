<?hh


<<__EntryPoint>>
function main_print_r() :mixed{
$xml = <<<XML
<root>
  <A>Hello</A>
  <B prop="val">World</B>
</root>
XML;

$doc = new DOMDocument;
$doc->loadXML($xml);
echo "--Document--\n";
print_r($doc);
echo "--Element--\n";
print_r($doc->documentElement);
}
