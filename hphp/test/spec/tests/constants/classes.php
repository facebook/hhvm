<?php

error_reporting(-1);

/*
abstract class AC {}

new AC;     // Cannot instantiate abstract class AC
*/

interface i1 {}
interface i2 {}
class C1 {}
class C2 extends c1 implements i1, i2 {}

$c = new C2;
var_dump($c);
