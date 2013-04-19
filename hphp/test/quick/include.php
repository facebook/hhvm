<?php

$a = 1;#"a\n";
print $a."\n";

require 'include.1.inc';

print $a."\n";
print $b."\n";

function foo() {
  require 'include.2.inc';
}
foo();

$path = dirname(__FILE__) . '/include.3.inc';
require $path;

$path = __DIR__ . '/include.3.inc';
require $path;

if (0) {
  // to ensure we include the file in
  // RepoAuthoritative mode
  require 'include.3.inc';
}
