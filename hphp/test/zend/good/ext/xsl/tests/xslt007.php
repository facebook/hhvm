<?hh


<<__EntryPoint>>
function main_entry(): void {
  echo "Test 7: Transform To Uri";
  include("prepare.inc");
  $proc = XSLTPrepare::getProc();
  $proc->importStylesheet(XSLTPrepare::getXSL());
  print "\n";
  $doc = $proc->transformToUri(XSLTPrepare::getDOM(), "file://".dirname(__FILE__)."/out.xml");
  print file_get_contents(dirname(__FILE__)."/out.xml");
  unlink(dirname(__FILE__)."/out.xml");
  print "\n";
}
