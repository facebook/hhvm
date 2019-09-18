<?hh

//var_dump($proc->registerPHPFunctions(array()));
//var_dump($proc->transformToXml($dom));

<<__EntryPoint>>
function main_entry(): void {
  include dirname(__FILE__) .'/prepare.inc';
  $phpfuncxsl = new domDocument();
  $phpfuncxsl->load(dirname(__FILE__)."/phpfunc.xsl");
  if(!$phpfuncxsl) {
    echo "Error while parsing the xsl document\n";
    exit;
  }
  $proc->importStylesheet($phpfuncxsl);
  var_dump($proc->registerPHPFunctions());
  var_dump($proc->transformToXml($dom));
}
