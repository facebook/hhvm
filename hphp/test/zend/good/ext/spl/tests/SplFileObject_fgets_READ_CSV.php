<?php
$f = new SplFileObject("data://,a,b,c,line1\n");
$f->setFlags(SplFileObject::READ_CSV);
var_dump($f->fgets());
