<?hh
<<__EntryPoint>>
function entrypoint_xsltprocessor_registerPHPFunctionsfuncundef(): void {
  include dirname(__FILE__) .'/prepare.inc';
  $phpfuncxsl = new domDocument();
  $phpfuncxsl->load(dirname(__FILE__)."/phpfunc-undef.xsl");
  if(!$phpfuncxsl) {
    echo "Error while parsing the xsl document\n";
    exit;
  }
  $proc = XSLTPrepare::getProc();
  $proc->importStylesheet($phpfuncxsl);
  var_dump($proc->registerPHPFunctions());
  var_dump($proc->transformToXML(XSLTPrepare::getDOM()));
}
