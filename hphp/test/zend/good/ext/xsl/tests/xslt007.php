<?hh


<<__EntryPoint>>
function main_entry(): void {
  echo "Test 7: Transform To Uri";
  include("prepare.inc");
  $proc = XSLTPrepare::getProc();
  $proc->importStylesheet(XSLTPrepare::getXSL());
  print "\n";
  $tmpfile = __SystemLib\hphp_test_tmppath('out.xml');
  $doc = $proc->transformToURI(XSLTPrepare::getDOM(), "file://$tmpfile");
  print file_get_contents($tmpfile);
  unlink($tmpfile);
  print "\n";
}
