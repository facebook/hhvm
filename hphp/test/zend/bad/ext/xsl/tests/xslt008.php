<?php
echo "Test 8: Stream Wrapper Includes ";
include("prepare.inc");
$xsl = new domDocument;
$xsl->load(dirname(__FILE__)."/streamsinclude.xsl");
if(!$xsl) {
  echo "Error while parsing the document\n";
  exit;
}
chdir(dirname(__FILE__));
$proc->importStylesheet($xsl);
print "\n";
print $proc->transformToXML($dom);

