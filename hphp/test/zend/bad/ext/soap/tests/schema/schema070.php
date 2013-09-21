<?php
include "test_schema.inc";
$schema = <<<EOF
	<complexType name="testType">
		<attribute name="str" type="string"/>
		<attributeGroup ref="tns:int_group"/>
	</complexType>
	<attributeGroup name="int_group">
		<attribute name="int" type="int" default="5"/>
	</attributeGroup>
EOF;
test_schema($schema,'type="tns:testType"',(object)array("str"=>"str"));
echo "ok";
?>