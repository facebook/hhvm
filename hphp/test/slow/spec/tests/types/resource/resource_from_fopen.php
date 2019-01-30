<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

$infile = fopen("Testfile.txt", 'r');
var_dump($infile);
echo "\n";
print_r($infile);
echo "\n";

$infile = fopen("NoSuchFile.txt", 'r');
var_dump($infile);

$infile = @fopen("NoSuchFile.txt", 'r');
var_dump($infile);
