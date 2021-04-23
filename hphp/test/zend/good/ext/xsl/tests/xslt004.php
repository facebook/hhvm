<?hh
<<__EntryPoint>> function main(): void {
echo "Test 4: Checking UTF8 Output";
include("prepare.inc");
$xsl = XSLTPrepare::getXSL();
$xp = new domxpath($xsl);
$res = $xp->query("/xsl:stylesheet/xsl:output/@encoding");
if ($res->length != 1) {
    print "No or more than one xsl:output/@encoding found";
    exit;
}
$res->item(0)->value = "utf-8";
$proc = XSLTPrepare::getProc();
$proc->importStylesheet($xsl);
print "\n";
print $proc->transformToXML(XSLTPrepare::getDOM());
print "\n";
}
