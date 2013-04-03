<?php

namespace foo;

define('foo', 3);

const foo = 1;

class foo {
	const foo = 2;
}

interface Ifoo {
	const foo = 4;
}

$const  = __NAMESPACE__ .'\\foo';  // class
$const2 = __NAMESPACE__ .'\\Ifoo'; // interface

var_dump(	foo,
			\foo\foo, 
			namespace\foo, 
			\foo\foo::foo, 
			$const::foo,
			\foo,
			constant('foo'),
			Ifoo::foo,
			$const2::foo
			);

?>