<?php
for( $i=1; $i<=64; $i = $i*2 ){
  echo 'Input: '. $i . PHP_EOL;
  $random = mcrypt_create_iv( $i, MCRYPT_DEV_URANDOM );
  echo ' Length: ' . strlen( $random ) . PHP_EOL;
  echo ' Hex: '. bin2hex( $random ) . PHP_EOL;
  echo PHP_EOL;
}
?>