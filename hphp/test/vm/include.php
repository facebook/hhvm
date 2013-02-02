<?php

$a = 1;#"a\n";
print $a."\n";

require 'include_b.php';

print $a."\n";
print $b."\n";

function foo() {
  require 'include_c.php';
}
foo();

$path = dirname(__FILE__) . '/include_d.php';
require $path;

$path = __DIR__ . '/include_d.php';
require $path;

