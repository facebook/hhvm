<?php

$fname = tempnam('/tmp', 'foobar');
file_put_contents($fname, 'herpderp');
$spl = new SplFileObject($fname, 'r');
$spl->fseek($spl->getSize() - 4);
var_dump($spl->fgets());
