<?hh

function reproduce( $code, $func ) {
        eval( $code );
        return $func( varray[] );
}


<<__EntryPoint>>
function main_eval_closure() {
echo reproduce('function foo() {
    return (function() {return "first\n";})();
}', 'foo');
echo reproduce('function bar() {
    return (function() {return "second\n";})();
}', 'bar');
}
