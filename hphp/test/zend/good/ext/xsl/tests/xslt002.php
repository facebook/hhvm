<?php
echo "Test 2: Transform To HTML String";
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

// changing output method to html
$xp = new domxpath($xsl);
$res = $xp->query("/xsl:stylesheet/xsl:output/@method");
if ($res->length != 1) {
    print "No or more than one xsl:output/@method found";
    exit;
}
$res->item(0)->value = "html";
$proc->importStylesheet($xsl);
print "\n";
print $proc->transformToXml($dom);
print "\n";

?>
