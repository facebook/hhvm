<?hh
<<__EntryPoint>> function main(): void {
echo "Test 2: Transform To HTML String";
include("prepare.inc");
// changing output method to html
$xsl = XSLTPrepare::getXSL();
$xp = new domxpath($xsl);
$res = $xp->query("/xsl:stylesheet/xsl:output/@method");
if ($res->length != 1) {
    print "No or more than one xsl:output/@method found";
    exit;
}
$res->item(0)->value = "html";
$proc = XSLTPrepare::getProc();
$proc->importStylesheet($xsl);
print "\n";
print $proc->transformToXML(XSLTPrepare::getDOM());
print "\n";
}
