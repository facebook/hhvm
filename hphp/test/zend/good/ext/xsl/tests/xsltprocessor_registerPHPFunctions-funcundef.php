<?php
include dirname(__FILE__) .'/prepare.inc';
$phpfuncxsl = new domDocument();
$phpfuncxsl->load(dirname(__FILE__)."/phpfunc-undef.xsl");
if(!$phpfuncxsl) {
  echo "Error while parsing the xsl document\n";
  exit;
}
$proc->importStylesheet($phpfuncxsl);
var_dump($proc->registerPHPFunctions());
var_dump($proc->transformToXml($dom));