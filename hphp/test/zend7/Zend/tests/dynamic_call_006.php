<?php
function test() {
    $func = 'extract';
    $func(['a' => 'b']);

    $func = 'compact';
    $func(['a']);

    $func = 'parse_str';
    $func('a=b');

    $func = 'get_defined_vars';
    $func();

    $func = 'assert';
    $func('1==2');

    $func = 'func_get_args';
    $func();

    $func = 'func_get_arg';
    $func(1);

    $func = 'func_num_args';
    $func();
}
test();

?>
