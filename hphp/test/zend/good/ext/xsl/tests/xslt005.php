<?php
echo "Test 5: Checking Indent";
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

$xp = new domxpath($xsl);
$res = $xp->query("/xsl:stylesheet/xsl:output/@indent");
if ($res->length != 1) {
    print "No or more than one xsl:output/@indent found";
    exit;
}
$res->item(0)->value = "yes";
$proc->importStylesheet($xsl);
print "\n";
print $proc->transformToXml($dom);
print "\n";

?>
