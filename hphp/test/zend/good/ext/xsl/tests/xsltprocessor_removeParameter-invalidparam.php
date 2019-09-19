<?hh
<<__EntryPoint>>
function main_entry(): void {
  include dirname(__FILE__) .'/prepare.inc';
  $proc->importStylesheet($xsl);
  var_dump($proc->removeParameter('', 'doesnotexist'));
}
