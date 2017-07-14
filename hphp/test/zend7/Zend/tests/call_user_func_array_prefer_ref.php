<?php

namespace {
    call_user_func_array('array_multisort', [[3, 2, 1]]);

    $args = [[3, 2, 1]];
    call_user_func_array('array_multisort', $args);
    var_dump($args);
    unset($args);

    $array = [3, 2, 1];
    call_user_func('array_multisort', $array);
    var_dump($array);
    unset($array);
}

namespace Foo {
    call_user_func_array('array_multisort', [[3, 2, 1]]);

    $args = [[3, 2, 1]];
    call_user_func_array('array_multisort', $args);
    var_dump($args);
    unset($args);

    $array = [3, 2, 1];
    call_user_func('array_multisort', $array);
    var_dump($array);
    unset($array);
}

?>
