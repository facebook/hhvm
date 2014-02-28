<?php
include dirname(__FILE__) .'/prepare.inc';
var_dump($proc->getParameter('', 'doesnotexist'));