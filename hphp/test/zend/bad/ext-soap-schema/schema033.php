<?php
include "test_schema.inc";
$schema = <<<EOF
	<complexType name="testType2">
		<sequence>
			<element name="int" type="int"/>
		</sequence>
	</complexType>
	<complexType name="testType">
		<sequence>
			<element name="int" type="int"/>
			<element name="nest" type="tns:testType2"/>
		</sequence>
	</complexType>
EOF;
test_schema($schema,'type="tns:testType"',(object)array("int"=>123.5,"nest"=>array("int"=>123.5)));
echo "ok";
?>