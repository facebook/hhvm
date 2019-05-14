<?php <<__EntryPoint>> function main() {
$fp = fopen (__FILE__, 'r');
var_dump(fstat( $fp ) );
fclose($fp);
echo "===DONE===\n";
}
