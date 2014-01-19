<?php

$sxe = new SimpleXMLElement('<foo />');
$sxe->addChild('options');
$sxe->options->addChild('paddingtop', 0);
echo 'Success
';
