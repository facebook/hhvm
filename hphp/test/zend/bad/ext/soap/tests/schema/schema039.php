<?php
include "test_schema.inc";
$schema = <<<EOF
	<complexType name="testType">
		<sequence>
			<element name="str" type="string"/>
		</sequence>
		<attributeGroup ref="tns:intGroup"/>
	</complexType>
	<attributeGroup name="intGroup">
		<attribute name="int" type="int"/>
	</attributeGroup>
EOF;
test_schema($schema,'type="tns:testType"',(object)array("str"=>"str","int"=>123.5));
echo "ok";
?>