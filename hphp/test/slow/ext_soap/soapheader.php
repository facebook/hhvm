<?php


<<__EntryPoint>>
function main_soapheader() {
$soapHeader = new SoapHeader("env", "header1", ['foo' => 'bar']);
var_dump($soapHeader);
var_dump($soapHeader->namespace);
var_dump($soapHeader->name);
var_dump($soapHeader->data);
var_dump($soapHeader->mustUnderstand);

$soapHeader->namespace = "namespace";
$soapHeader->name = "name";
$soapHeader->data = array("hello" => 'world');
$soapHeader->mustUnderstand = true;

var_dump($soapHeader);

unset($soapHeader->data);

var_dump($soapHeader);
}
