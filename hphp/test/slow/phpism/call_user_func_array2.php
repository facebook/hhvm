<?hh
function exampleFunc($arg1, $arg2) {
    echo($arg1);
    echo($arg2);
}

call_user_func_array('exampleFunc', array("one", "two"));
