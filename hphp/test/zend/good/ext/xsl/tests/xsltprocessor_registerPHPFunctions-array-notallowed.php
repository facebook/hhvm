<?hh
<<__EntryPoint>>
function entrypoint_xsltprocessor_registerPHPFunctionsarraynotallowed(
): void {
  include dirname(__FILE__) .'/prepare.inc';
  $phpfuncxsl = new domDocument();
  $phpfuncxsl->load(dirname(__FILE__)."/phpfunc.xsl");
  if(!$phpfuncxsl) {
    echo "Error while parsing the xsl document\n";
    exit;
  }
  $proc = XSLTPrepare::getProc();
  $proc->importStylesheet($phpfuncxsl);
  var_dump($proc->registerPHPFunctions(varray[]));
  var_dump($proc->transformToXml(XSLTPrepare::getDOM()));
}
