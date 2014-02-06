<?php
echo "Test 8: Stream Wrapper Includes ";
//include("prepare.inc");
$dom = new domDocument;
$dom->load(dirname(__FILE__)."/xslt.xml");
if(!$dom) {
      echo "Error while parsing the document\n";
        exit;
}
$xsl = new domDocument;
$xsl->load(dirname(__FILE__)."/xslt.xsl");
if(!$xsl) {
      echo "Error while parsing the document\n";
        exit;
}
$proc = new xsltprocessor;
if(!$proc) {
      echo "Error while making xsltprocessor object\n";
        exit;
}

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

?>
