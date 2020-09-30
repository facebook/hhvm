<?hh // partial
//This used to infinitely loop; now it should terminate
function __toString(){switch ( $a ? $b : $c ) ): default: $a; endswitch;}
