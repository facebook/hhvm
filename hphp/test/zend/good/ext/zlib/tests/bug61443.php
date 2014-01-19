<?php
ob_start(); echo "foo\n"; ob_get_clean(); 
if(!headers_sent()) ini_set('zlib.output_compression', true); echo "end\n";
?>
DONE