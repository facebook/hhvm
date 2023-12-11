<?hh
<<__EntryPoint>>
function entrypoint_xsltprocessor_setparameternostring(): void {
  include dirname(__FILE__) .'/prepare.inc';
  $proc = XSLTPrepare::getProc();
  $proc->importStylesheet(XSLTPrepare::getXSL());
  var_dump($proc->setParameter('', vec[4, 'abc']));
  $proc->transformToXML(XSLTPrepare::getDOM());
}
