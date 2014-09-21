<?php

function ref(&$a) {}

$rfunc = new ReflectionFunction('ref');
$rparam = $rfunc->getParameters();
echo $rparam[0];
