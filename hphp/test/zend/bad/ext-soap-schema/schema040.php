<?php
include "test_schema.inc";
$schema = <<<EOF
	<complexType name="testType">
		<sequence>
			<element name="str" type="string"/>
		</sequence>
		<attribute name="int">
			<simpleType>
				<restriction base="int"/>
			</simpleType>
		</attribute>
	</complexType>
EOF;
test_schema($schema,'type="tns:testType"',(object)array("str"=>"str","int"=>123.5));
echo "ok";
?>