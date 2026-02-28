<?hh
function exampleFunc($arg1) :mixed{
    echo($arg1);
}
<<__EntryPoint>> function main(): void {
call_user_func(exampleFunc<>, 'one');
}
