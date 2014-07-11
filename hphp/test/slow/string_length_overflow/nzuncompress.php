<?php

mt_srand(0);
$s = '';
for ( $i = 0; $i < 1000000; $i++ ) {
  $s .= chr(mt_rand(0,255));
}
$c = nzcompress($s);
$c = substr_replace($c, pack('N', (1<<32)-1), 4, 4);
nzuncompress($c);
print "Done\n";
