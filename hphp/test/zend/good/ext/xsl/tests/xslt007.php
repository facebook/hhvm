<?php
echo "Test 7: Transform To Uri";
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

$proc->importStylesheet($xsl);
print "\n";
$doc = $proc->transformToUri($dom, "file://".dirname(__FILE__)."/out.xml");
print file_get_contents(dirname(__FILE__)."/out.xml");
unlink(dirname(__FILE__)."/out.xml");
print "\n";

?>
