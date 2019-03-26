<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

///*

// take an undefined parameter passed byRef, and pass it byRef where
// the underlying value is modified. This causes the previously undefined variable
// to now be defined.

function f(&$p)
{
   echo '$p '.(isset($p) ? "is set\n" : "is not set\n");
   g(&$p);
   echo '$p '.(isset($p) ? "is set\n" : "is not set\n");
   var_dump($p);
}

function g(&$q)
{
   echo '$q '.(isset($q) ? "is set\n" : "is not set\n");
   $q = -10;
}

var_dump($x);
f(&$x);           // non-existant variable going in
var_dump($x);

$a = array(10, 20, 30);
var_dump($a);

f(&$a[0]);
var_dump($a);

f(&$a[5]);      // non-existant element going in
var_dump($a);

f(&$a["red"]);  // non-existant element going in
var_dump($a);
//*/
///*

// take an undefined parameter passed byRef. This causes the previously
// undefined variable to now be defined with a value of NULL.

function h(&$p)
{
   echo '$p '.(isset($p) ? "is set\n" : "is not set\n");
   var_dump($p);
}

var_dump($x);
h(&$x);           // non-existant variable going in
var_dump($x);

$a = array(10, 20, 30);
var_dump($a);

h(&$a[0]);
var_dump($a);

h(&$a[5]);      // non-existant element going in
var_dump($a);

h(&$a["red"]);  // non-existant element going in
var_dump($a);
//*/
