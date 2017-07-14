<?php
$a = new stdClass;
$a->{1234} = "Numeric";
$a->a1234 = "String";

$properties = get_object_vars($a);
var_dump(array_key_exists(1234, $properties));
echo "Value: {$properties[1234]}\n";

?>
