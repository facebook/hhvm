<?hh
<<__EntryPoint>>
function main_entry(): void {
  include dirname(__FILE__) .'/prepare.inc';
  var_dump($proc->getParameter('', 'doesnotexist'));
}
