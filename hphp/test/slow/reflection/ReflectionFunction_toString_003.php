<?php

function zeroArgs(){}

$rfunc = new ReflectionFunction('zeroArgs');
echo $rfunc;
