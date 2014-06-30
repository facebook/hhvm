<?php

// Boundary case truth table for string functions with start and length
// parameters similar to substr()

function print_result( $desc, $inLen, $start, $scanLen, $result ) {
  printf( "%-20s %-6d %-6d %-6d %s\n",
    $desc, $inLen, $start, $scanLen,
    $result === false ? 'false' : $result );
}

printf( "%-20s %-6s %-6s %-6s %s\n", "Test", "in", "start", "len", "result" );

for ( $inLen = 0; $inLen <= 2; $inLen++ ) {
  $xxx = str_repeat( 'x', $inLen );
  $abc = substr( "abcdefg", 0, $inLen );
  for ( $start = -$inLen - 1; $start <= $inLen + 1; $start++ ) {
    for ( $scanLen = -$inLen - 1; $scanLen <= $inLen + 1; $scanLen++ ) {
      print_result( "strspn(x,x)", $inLen, $start, $scanLen,
        strspn( $xxx, 'x', $start, $scanLen ) );
      print_result( "strspn(x,y)", $inLen, $start, $scanLen,
        strspn( $xxx, 'y', $start, $scanLen ) );
      print_result( "strcspn(x,x)", $inLen, $start, $scanLen,
        strcspn( $xxx, 'x', $start, $scanLen ) );
      print_result( "strcspn(x,y)", $inLen, $start, $scanLen,
        strcspn( $xxx, 'y', $start, $scanLen ) );
      print_result( "substr_replace", $inLen, $start, $scanLen,
        substr_replace( $xxx, 'y', $start, $scanLen ) );
      print_result( "substr", $inLen, $start, $scanLen,
        substr( $abc, $start, $scanLen ) );
    }
  }
}
