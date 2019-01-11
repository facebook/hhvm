<?php
error_reporting(E_ALL & ~E_NOTICE);   // We don't want the notice for $undefined
$arr = array('a'=>$undefined);
$result = extract(&$arr, EXTR_REFS);
var_dump(array($a));
echo "Done\n";
?>
