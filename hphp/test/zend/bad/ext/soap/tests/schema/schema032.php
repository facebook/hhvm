<?php
include "test_schema.inc";
$schema = <<<EOF
	<complexType name="testType">
		<choice>
			<element name="int" type="int"/>
			<element name="str" type="string"/>
		</choice>
	</complexType>
EOF;
test_schema($schema,'type="tns:testType"',(object)array("int"=>123.5));
echo "ok";
?>