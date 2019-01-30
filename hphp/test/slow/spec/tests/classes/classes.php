<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

interface i1 {}
interface i2 {}
class C1 {}
class C2 extends c1 implements i1, i2 {}

$c = new C2;
var_dump($c);
