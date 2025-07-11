<?hh

function reproduce( $code, $func ) :mixed{
        eval( $code );
        return HH\dynamic_fun($func)( vec[] );
}


<<__EntryPoint>>
function main_eval_closure() :mixed{
echo reproduce('<<__DynamicallyCallable>> function foo() {
    return (function() {return "first\n";})();
}', 'foo');
echo reproduce('<<__DynamicallyCallable>> function bar() {
    return (function() {return "second\n";})();
}', 'bar');
}
