<?php

namespace ShutdownTest;

function test() {
  var_dump('function');
}

class Test {
    function handleInstance() {
        var_dump('Method - instance');
    }

    static function handleStatic() {
        var_dump('Method - static');
    }
}

register_postsend_function(__NAMESPACE__ . '\test');
register_postsend_function([new Test, 'handleInstance']);
register_postsend_function([__NAMESPACE__ . '\Test', 'handleStatic']);
register_postsend_function(function () {
    var_dump('Lambda');
});


register_postsend_function(function () {
    var_dump(func_get_args());
}, 123);
register_postsend_function(function () {
    var_dump(func_get_args());
}, ['foo' => 'bar'], 123);
register_postsend_function(function () {
    var_dump(func_get_args());
}, (object) ['foo' => 'bar']);
