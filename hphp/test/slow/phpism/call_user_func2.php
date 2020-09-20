<?hh
function exampleFunc($arg1) {
    echo($arg1);
}
<<__EntryPoint>> function main(): void {
call_user_func(fun('exampleFunc'), 'one');
}
