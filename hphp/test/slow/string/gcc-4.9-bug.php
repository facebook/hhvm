<?php

// https://github.com/facebook/hhvm/issues/8011

class Test
{
    public function foo()
    {
        $x = ['foo', 'bar'];
        // The actual operation isn't important, just need to do something to turn the literal
        // strings into refcounted strings
        $y = array_map(function($it) { return $it.$it; }, $x);
        return array_unique($y, SORT_STRING);
    }
}

var_dump((new Test())->foo());
