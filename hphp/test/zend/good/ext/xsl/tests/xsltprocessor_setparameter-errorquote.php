<?hh
<<__EntryPoint>>
function entrypoint_xsltprocessor_setparametererrorquote(): void {
  include dirname(__FILE__) .'/prepare.inc';
  $proc = XSLTPrepare::getProc();
  $proc->importStylesheet(XSLTPrepare::getXSL());
  $proc->setParameter('', '', '"\'');
  $proc->transformToXML(XSLTPrepare::getDOM());
}
