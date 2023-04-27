<?hh <<__EntryPoint>> function main(): void {
$dom = new DOMDocument;
$dom->load(dirname(__FILE__)."/area_name.xml");
if(!$dom) {
  echo "Error while parsing the document\n";
  exit;
}
$xsl = new DOMDocument;
$xsl->load(dirname(__FILE__)."/area_list.xsl");
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
print $proc->transformToXML($dom);

//this segfaulted before
print $dom->documentElement->firstChild->nextSibling->nodeName;
}
