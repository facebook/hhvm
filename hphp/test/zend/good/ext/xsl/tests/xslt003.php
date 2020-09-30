<?hh


<<__EntryPoint>>
function main_entry(): void {
  echo "Test 3: Using Parameters";
  include("prepare.inc");
  $proc = XSLTPrepare::getProc();
  $proc->importStylesheet(XSLTPrepare::getXSL());
  $proc->setParameter( "", "foo","hello world");
  print "\n";
  print $proc->transformToXML(XSLTPrepare::getDOM());
  print "\n";
}
