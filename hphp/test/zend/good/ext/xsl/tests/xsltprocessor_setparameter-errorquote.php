<?php
include dirname(__FILE__) .'/prepare.inc';
$proc->importStylesheet($xsl);
$proc->setParameter('', '', '"\'');
$proc->transformToXml($dom);