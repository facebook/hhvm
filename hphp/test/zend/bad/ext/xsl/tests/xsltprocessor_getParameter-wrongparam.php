<?php
include dirname(__FILE__) .'/prepare.inc';
var_dump($proc->getParameter());
var_dump($proc->getParameter(array(), array()));
var_dump($proc->getParameter('', array()));