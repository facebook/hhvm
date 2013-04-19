<?php
namespace a\b\c;

use a\b\c as test;

require "ns_022.inc";

function foo() {
	echo __FUNCTION__,"\n";
}

test\foo();
\test::foo();