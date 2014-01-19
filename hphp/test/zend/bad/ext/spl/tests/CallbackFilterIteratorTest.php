<?php

class A {
    function test($value, $key, $inner) {
        return test($value, $key, $inner);
    }
}

class B {
    static function test($value, $key, $inner) {
        return test($value, $key, $inner);
    }
}

function test($value, $key, $inner) {
	printf("%s / %s / %d / %d\n"
		, $value
		, $key
		, $value == $inner->current()
		, $key == $inner->key()
	);
	return $value === 1 || $value === 4;
}

$tests = array(
    'instance method'    => function() { return array(new A, 'test'); },
    'static method'      => function() { return array('B', 'test'); },
    'static method (2)'  => function() { return 'B::test'; },
    'function'           => function() { return 'test'; },
    'anonymous function' => function() { return function($value, $key, $inner) { return test($value, $key, $inner); }; },
);

foreach($tests as $name => $test) {

    $callback = $test();
    $it = new ArrayIterator(range(1, 5));
    $it = new CallbackFilterIterator($it, $callback);

    echo " = $name =\n";

    foreach($it as $value) {
        echo "=> $value\n";
    }

	// same test, with no reference to callback

    $it = new ArrayIterator(range(1, 5));
    $it = new CallbackFilterIterator($it, $test());
    unset($callback);

    foreach($it as $value) {
        echo "=> $value\n";
    }
}