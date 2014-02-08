<?php
echo "Test 9: Stream Wrapper XPath-Document()";
include("prepare.inc");

$xsl = new domDocument;
$xsl->load(dirname(__FILE__)."/documentxpath.xsl");
if(!$xsl) {
  echo "Error while parsing the document\n";
  exit;
}

$proc->importStylesheet($xsl);
print "\n";
print $proc->transformToXML($dom);

