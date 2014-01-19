<?php
include "test_schema.inc";
$schema = <<<EOF
	<element name="testType2">
		<complexType>
			<sequence>
				<element name="int" type="int"/>
			</sequence>
		</complexType>
	</element>
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