<?php
error_reporting(E_ALL & ~E_NOTICE);   // We don't want the notice for $undefined
$result = extract(array('a'=>$undefined), EXTR_REFS); 
var_dump(array($a));
echo "Done\n";
?>