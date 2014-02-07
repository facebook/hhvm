<?php
echo "Test 5: Checking Indent";
include("prepare.inc");
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

