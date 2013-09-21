<?php

function v($val) {
  $val = "Val changed";
}

function r(&$ref) {
  $ref = "Ref changed";
}


function vv($val1, $val2) {
  $val1 = "Val1 changed";
  $val2 = "Val2 changed";
}

function vr($val, &$ref) {
  $val = "Val changed";
  $ref = "Ref changed";
}

function rv(&$ref, $val) {
  $val = "Val changed";
  $ref = "Ref changed";
}

function rr(&$ref1, &$ref2) {
  $ref1 = "Ref1 changed";
  $ref2 = "Ref2 changed";
}


class C {

	function __construct($val, &$ref) {
	  $val = "Val changed";
	  $ref = "Ref changed";
	}

	function v($val) {
	  $val = "Val changed";
	}
	
	function r(&$ref) {
	  $ref = "Ref changed";
	}
	
	function vv($val1, $val2) {
	  $val1 = "Val1 changed";
	  $val2 = "Val2 changed";
	}
	
	function vr($val, &$ref) {
	  $val = "Val changed";
	  $ref = "Ref changed";
	}
	
	function rv(&$ref, $val) {
	  $val = "Val changed";
	  $ref = "Ref changed";
	}
	
	function rr(&$ref1, &$ref2) {
	  $ref1 = "Ref1 changed";
	  $ref2 = "Ref2 changed";
	}

} 

echo "\n ---- Pass by ref / pass by val: functions ----\n";
unset($u1, $u2);
v($u1);
r($u2);
var_dump($u1, $u2);

unset($u1, $u2);
vv($u1, $u2);
var_dump($u1, $u2);

unset($u1, $u2);
vr($u1, $u2);
var_dump($u1, $u2);

unset($u1, $u2);
rv($u1, $u2);
var_dump($u1, $u2);

unset($u1, $u2);
rr($u1, $u2);
var_dump($u1, $u2);


echo "\n\n ---- Pass by ref / pass by val: static method calls ----\n";
unset($u1, $u2);
C::v($u1);
C::r($u2);
var_dump($u1, $u2);

unset($u1, $u2);
C::vv($u1, $u2);
var_dump($u1, $u2);

unset($u1, $u2);
C::vr($u1, $u2);
var_dump($u1, $u2);

unset($u1, $u2);
C::rv($u1, $u2);
var_dump($u1, $u2);

unset($u1, $u2);
C::rr($u1, $u2);
var_dump($u1, $u2);

echo "\n\n ---- Pass by ref / pass by val: instance method calls ----\n";
unset($u1, $u2);
$c = new C($u1, $u2);
var_dump($u1, $u2);

unset($u1, $u2);
$c->v($u1);
$c->r($u2);
var_dump($u1, $u2);

unset($u1, $u2);
$c->vv($u1, $u2);
var_dump($u1, $u2);

unset($u1, $u2);
$c->vr($u1, $u2);
var_dump($u1, $u2);

unset($u1, $u2);
$c->rv($u1, $u2);
var_dump($u1, $u2);

unset($u1, $u2);
$c->rr($u1, $u2);
var_dump($u1, $u2);

?>