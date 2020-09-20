<?hh // partial
//This used to infinitely loop; now it should terminate
function __tostring(){switch ( $a ? $b : $c ) ): default: $a; endswitch;}
