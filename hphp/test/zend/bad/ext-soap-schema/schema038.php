<?php
include "test_schema.inc";
$schema = <<<EOF
	<complexType name="testType">
		<sequence>
			<element name="str" type="string"/>
		</sequence>
		<attribute ref="tns:int"/>
	</complexType>
	<attribute name="int" type="int"/>
EOF;
test_schema($schema,'type="tns:testType"',(object)array("str"=>"str","int"=>123.5));
echo "ok";
?>