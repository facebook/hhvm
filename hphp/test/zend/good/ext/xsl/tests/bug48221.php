<?hh
<<__EntryPoint>>
function entrypoint_bug48221(): void {
  include('prepare.inc');
  $proc = XSLTPrepare::getProc();
  $proc->importStylesheet(XSLTPrepare::getXSL());
  $proc->setParameter('', '', '"\'');
  $proc->transformToXML(XSLTPrepare::getDOM());
}
