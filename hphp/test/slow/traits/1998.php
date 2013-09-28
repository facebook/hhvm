<?php

trait Test {
  public function __construct() {
 }
  public function __destruct() {
 }
  public function func() {
 }
}
$rconstr = new ReflectionMethod('Test::__construct');
$rdestr = new ReflectionMethod('Test::__destruct');
$rfunc = new ReflectionMethod('Test::func');
var_dump($rconstr->isConstructor());
var_dump($rconstr->isDestructor());
var_dump($rdestr->isConstructor());
var_dump($rdestr->isDestructor());
var_dump($rfunc->isConstructor());
var_dump($rfunc->isDestructor());
