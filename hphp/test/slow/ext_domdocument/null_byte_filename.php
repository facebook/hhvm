<?php

$doc = new DOMDocument();
$doc->load('/etc/fonts/fonts.conf' . chr(0) . 'somethingelse.xml');
echo $doc->saveXML();
