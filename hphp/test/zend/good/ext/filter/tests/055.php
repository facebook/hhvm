<?php
$values = Array(
	array("01-23-45-67-89-ab", null),
	array("01-23-45-67-89-ab", array("options" => array("separator" => "-"))),
	array("01-23-45-67-89-ab", array("options" => array("separator" => "."))),
	array("01-23-45-67-89-ab", array("options" => array("separator" => ":"))),
	array("01-23-45-67-89-AB", null),
	array("01-23-45-67-89-aB", null),
	array("01:23:45:67:89:ab", null),
	array("01:23:45:67:89:AB", null),
	array("01:23:45:67:89:aB", null),
	array("01:23:45-67:89:aB", null),
	array("xx:23:45:67:89:aB", null),
	array("0123.4567.89ab", null),
	array("01-23-45-67-89-ab", array("options" => array("separator" => "--"))),
	array("01-23-45-67-89-ab", array("options" => array("separator" => ""))),
);
foreach ($values as $value) {
	var_dump(filter_var($value[0], FILTER_VALIDATE_MAC, $value[1]));
}

echo "Done\n";
?>