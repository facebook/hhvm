<?php


echo "*** Test substituting argument 1 with float values ***\n";

$myUid = posix_getuid();

$myUid = $myUid - 1.1;

$variation_array = array(
  'float '.$myUid => $myUid,
  'float -'.$myUid => -$myUid,
  'float 12.3456789000e10' => 12.3456789000e10,
  'float -12.3456789000e10' => -12.3456789000e10,
  'float .5' => .5,
  );


foreach ( $variation_array as $var ) {
  var_dump(posix_setuid( $var  ) );
}
?>