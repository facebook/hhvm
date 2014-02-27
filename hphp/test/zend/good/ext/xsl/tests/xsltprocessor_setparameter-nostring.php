<?php
include dirname(__FILE__) .'/prepare.inc';
$proc->importStylesheet($xsl);
var_dump($proc->setParameter('', array(4, 'abc')));
$proc->transformToXml($dom);