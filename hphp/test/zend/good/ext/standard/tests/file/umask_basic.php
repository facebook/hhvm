<?php
/* Prototype: int umask ( [int $mask] );
   Description: Changes the current umask
*/

echo "*** Testing umask() : basic functionality ***\n";
// checking umask() on all the modes
for($mask = 0000; $mask <= 0777; $mask++) {
  echo "-- Setting umask to $mask --\n";
  var_dump( umask($mask) );
  var_dump( umask() );
  echo "\n";
  if ($mask != umask()) {
    die('An error occurred while changing back the umask');
  }
}

echo "Done\n";
?>