<?hh
function exampleFunc($arg1, $arg2) {
    echo($arg1);
    echo($arg2);
}
<<__EntryPoint>> function main(): void {
call_user_func_array(fun('exampleFunc'), array("one", "two"));
}
