<?hh <<__EntryPoint>> function main(): void {
echo "Test 12: Using Associative Array of Parameters";

$dom = new DOMDocument;
$dom->load(dirname(__FILE__)."/xslt.xml");
if(!$dom) {
  echo "Error while parsing the document\n";
  exit;
}

$xsl = new DOMDocument;
$xsl->load(dirname(__FILE__)."/xslt012.xsl");
if(!$xsl) {
  echo "Error while parsing the document\n";
  exit;
}

$proc = new XSLTProcessor;
if(!$proc) {
  echo "Error while making xsltprocessor object\n";
  exit;
}


$proc->importStylesheet($xsl);

$parameters = dict[
                    'foo' => 'barbar',
                    'foo1' => 'test',
                    ];

$proc->setParameter( "", $parameters);

print "\n";
print $proc->transformToXML($dom);
print "\n";
}
