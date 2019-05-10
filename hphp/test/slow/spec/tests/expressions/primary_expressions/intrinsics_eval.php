<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/
<<__EntryPoint>> function main() {
error_reporting(-1);

$str = "Hello";

eval("echo \$str . \"\\n\";");
}
