<?php

function reproduce( $code ) {
        $template = eval( $code );
        return $template( array() );
}


<<__EntryPoint>>
function main_eval_closure() {
echo reproduce('return function () {
    return (function() {return "first\n";})();
};');
echo reproduce('return function () {
    return (function() {return "second\n";})();
};');
}
