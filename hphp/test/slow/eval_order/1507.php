<?php

error_reporting(E_ALL & ~E_NOTICE);
function id($x,$y) {
 return $x;
 }
function id1($x) {
 return $x;
 }
function pid($x) {
 var_dump($x);
 return $x;
 }
class cls {
  function __construct() {
 print 'ctor
';
 }
  function f($x) {
 return $this;
 }
  function ttest() {
    return $this->f(pid('arg1'),pid('arg2'));
  }
}
$d = id1(new cls())  ->f('arg1')  ->f('arg2')  ->f('arg3');
$d = id1(new cls())  ->f('arg1', 'argex1')  ->f('arg2', 'argex2')  ->f('arg3', 'argex3');
$d = id(new cls(), pid('idarg'))  ->f(pid('arg1'), pid('argex1'))  ->f(pid('arg2'), pid('argex2'))  ->f(pid('arg3'), pid('argex3'));
$d->ttest();
