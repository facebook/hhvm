<?php

namespace foo\x;

const x = 2;

class x { 
	const x = 1;
}


var_dump(namespace\x,
x::x,
namespace\x::x);
var_dump(defined('foo\x\x'));

?>