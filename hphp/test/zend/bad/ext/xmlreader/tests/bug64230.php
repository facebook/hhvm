<?php
echo "Test\n";

function show_internal_errors() {
	foreach (libxml_get_errors() as $error) {
		printf("Internal: %s\n", $error->message);
	}
	libxml_clear_errors();
}

echo "Internal errors TRUE\n";
libxml_use_internal_errors(true);

$x = new XMLReader;
$x->xml("<root att/>");
$x->read();

show_internal_errors();

echo "Internal errors FALSE\n";
libxml_use_internal_errors(false);

$x = new XMLReader;
$x->xml("<root att/>");
$x->read();

show_internal_errors();

?>
Done