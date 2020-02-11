<?hh
include dirname(__FILE__) .'/prepare.inc';
$proc->importStylesheet($xsl);
var_dump($proc->setParameter('', varray[4, 'abc']));
$proc->transformToXml($dom);
