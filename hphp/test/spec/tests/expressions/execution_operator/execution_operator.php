<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2014 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

///*
// $result = `dir *.*`;
$result = `ls`;
var_dump($result);
//*/

///*
// $result = `dir *.* >dirlist.txt`;
$result = `ls >dirlist.txt`;
var_dump($result);
unlink('dirlist.txt');
//*/

///*
$d = "dir";
$f = "*.*";
// $result = `$d {$f}`;
$result = `ls`;
var_dump($result);
//*/

///*
$result = ``;
var_dump($result);

$result = `  `;
var_dump($result);
//*/
