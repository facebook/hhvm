<?php
$f = new SplFileObject("data://,line 1\n");
$f->setFlags(SplFileObject::DROP_NEW_LINE);
var_dump($f->fgets());
