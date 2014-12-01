<?php
$a = new stdClass();
$a->foo = 'bar';

$v = new SoapVar($a, SOAP_ENC_OBJECT);

var_dump($v, $v->enc_value->foo);
