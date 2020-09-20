<?hh


<<__EntryPoint>>
function main_entry(): void {
  echo "Test 6: Transform To Doc";
  include("prepare.inc");
  $proc = XSLTPrepare::getProc();
  $proc->importStylesheet(XSLTPrepare::getXSL());
  print "\n";
  $doc = $proc->transformToDoc(XSLTPrepare::getDOM());
  print $doc->saveXML();
  print "\n";
}
