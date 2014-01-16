<?php

function foo () {}
async function async_foo() {}

class Bar {
  public function foo () {}
  public async function asyncFoo() {}
}

var_dump((new ReflectionFunction('foo'))->isAsync());
var_dump((new ReflectionFunction('async_foo'))->isAsync());
var_dump((new ReflectionMethod('Bar::foo'))->isAsync());
var_dump((new ReflectionMethod('Bar::asyncFoo'))->isAsync());
var_dump((new ReflectionFunction(async function () {}))->isAsync());
