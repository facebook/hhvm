<?php

class A {
 private $b = 10;
 }
class B extends A {
 private $b = 100;
 }
apc_store('key', new B());
var_dump(apc_fetch('key'));
