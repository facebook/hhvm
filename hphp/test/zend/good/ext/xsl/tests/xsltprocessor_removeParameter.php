<?hh
<<__EntryPoint>>
function main_entry(): void {
  include dirname(__FILE__) .'/prepare.inc';
  $proc = XSLTPrepare::getProc();
  $proc->importStylesheet(XSLTPrepare::getXSL());
  $proc->setParameter('', 'key', 'value');
  $proc->removeParameter('', 'key');
  var_dump($proc->getParameter('', 'key'));
}
