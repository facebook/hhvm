<?hh
<<__EntryPoint>>
function main_entry(): void {
  include dirname(__FILE__) .'/prepare.inc';
  $proc->importStylesheet($xsl);
  $proc->setParameter('', 'key', 'value');
  $proc->removeParameter('', 'key');
  var_dump($proc->getParameter('', 'key'));
}
