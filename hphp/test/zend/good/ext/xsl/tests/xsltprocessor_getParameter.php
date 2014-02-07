<?php
include dirname(__FILE__) .'/prepare.inc';
$proc->importStylesheet($xsl);
$proc->setParameter('', 'key', 'value');
var_dump($proc->getParameter('', 'key'));