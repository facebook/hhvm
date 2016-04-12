<?php

/**
 * THIS TEST ISN'T ENTIRELY CORRECT.
 * See https://github.com/facebook/hhvm/issues/6624.
 *
 * The real expected output from this test should be:
 *
 * Starting bar()
 * Constructing new FooBar
 * Destructing FooBar
 * Caught Exception
 * Unsetting $x
 * Finishing bar()
 *
 * But due to the attached github bug the FooBar instance is sticking around
 * too long and not getting cleaned up until the end of the test.
 */
class FooBar implements Iterator {
    function __construct()   { echo "Constructing new FooBar\n"; }
    function __destruct()    { echo "Destructing FooBar\n"; }
    function current ()      { throw new Exception; }
    function key ()          { return 0; }
    function next ()         {}
    function rewind ()       {}
    function valid ()        { return true; }
}

function foo() {
    $f = new FooBar;
    yield from $f;
}

function bar() {
    echo "Starting bar()\n";
    $x = foo();
    try {
        $x->next();
        var_dump($x->current());
    } catch (Exception $e) {
        echo "Caught Exception\n";
    }
    echo "Unsetting \$x\n";
    unset($x);
    echo "Finishing bar()\n";
}

bar();
