<?php


<<__EntryPoint>>
function main_ext_xsl_null_byte() {
$file = '/etc/passwd'.chr(0).'asdf';

$doc = new DOMDocument();
$proc = new XSLTProcessor();
var_dump($proc->setProfiling($file));
var_dump($proc->transformToURI($doc, $file));
}
