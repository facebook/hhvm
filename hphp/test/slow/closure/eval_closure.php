<?hh

function reproduce( $code, $func ) :mixed{
        eval( $code );
        return $func( vec[] );
}


<<__EntryPoint>>
function main_eval_closure() :mixed{
echo reproduce('function foo() {
    return (function() {return "first\n";})();
}', 'foo');
echo reproduce('function bar() {
    return (function() {return "second\n";})();
}', 'bar');
}
