<?hh


<<__EntryPoint>>
function main_entry(): void {
  echo "Test 1: Transform To XML String";
  include("prepare.inc");
  $proc = XSLTPrepare::getProc();
  $proc->importStylesheet(XSLTPrepare::getXSL());
  print "\n";
  print $proc->transformToXML(XSLTPrepare::getDOM());
  print "\n";
}
