<?php
include "test_schema.inc";
$schema = <<<EOF
	<element name="testType2" type="tns:testType2"/>
	<complexType name="testType2">
		<sequence>
			<element name="int" type="int"/>
		</sequence>
	</complexType>
	<complexType name="testType">
		<sequence>
			<element name="int" type="int"/>
			<element ref="tns:testType2"/>
		</sequence>
	</complexType>
EOF;
test_schema($schema,'type="tns:testType"',(object)array("int"=>123.5,"testType2"=>array("int"=>123.5)));
echo "ok";
?>