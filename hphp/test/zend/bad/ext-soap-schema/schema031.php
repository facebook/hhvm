<?php
include "test_schema.inc";
$schema = <<<EOF
	<complexType name="testType">
		<all>
			<element name="int" type="int"/>
			<element name="str" type="string"/>
		</all>
	</complexType>
EOF;
test_schema($schema,'type="tns:testType"',(object)array("str"=>"str","int"=>123.5));
echo "ok";
?>