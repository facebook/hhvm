<?php
include dirname(__FILE__) .'/prepare.inc';
$proc->removeParameter();
$proc->removeParameter(array(), array());
$proc->removeParameter('', array());