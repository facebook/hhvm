<?php
include "test_schema.inc";
$schema = <<<EOF
	<complexType name="testType">
		<group ref="tns:testGroup"/>
	</complexType>
	<group name="testGroup">
		<sequence>
			<element name="int" type="int"/>
			<element name="str" type="string"/>
		</sequence>
	</group>
EOF;
test_schema($schema,'type="tns:testType"',(object)array("str"=>"str","int"=>123.5));
echo "ok";
?>