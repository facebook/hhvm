<?php
$chars = "\xE4";
setlocale(LC_CTYPE, "de_DE", "de", "german", "ge", "de_DE.ISO8859-1", "ISO8859-1");
echo strcasecmp($chars, strtoupper($chars))."\n";
echo stripos($chars, strtoupper($chars))."\n";
?>
