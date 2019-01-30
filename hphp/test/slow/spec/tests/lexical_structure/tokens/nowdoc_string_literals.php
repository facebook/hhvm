<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

$v = 123;

// test using unescaped ", embedded (actual) tab, variable substitution, multiple lines

$s = <<<      'ID'
S'o'me "\"t e\txt; \$v = $v"
Some more text
ID;
echo ">$s<\n\n";

var_dump(<<<'X'
X
);

var_dump(<<<'X'
xxx
yyy
X
);
