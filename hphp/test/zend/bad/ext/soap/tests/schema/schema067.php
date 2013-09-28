<?php
include "test_schema.inc";
$schema = <<<EOF
	<complexType name="testType">
		<attribute name="str" type="string"/>
		<attribute name="int" type="int" fixed="5"/>
	</complexType>
EOF;
test_schema($schema,'type="tns:testType"',(object)array("str"=>"str","int"=>5));
echo "ok";
?>