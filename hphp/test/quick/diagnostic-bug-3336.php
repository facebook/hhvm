<?php
// from: https://github.com/facebook/hhvm/issues/3336
function foo( $a, $b, $c = null, $d = null ) {
        echo "$a\n";
}

foo( 'works', 'fine' );

foo( 'fails' );
