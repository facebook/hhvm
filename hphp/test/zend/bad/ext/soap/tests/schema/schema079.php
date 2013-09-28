<?php
include "test_schema.inc";
$schema = <<<EOF
	<complexType name="testType">
		<sequence>
			<element name="int1" type="int"/>
			<element name="int2" type="int" form="qualified"/>
			<element name="int3" type="int" form="unqualified"/>
		</sequence>
	</complexType>
EOF;

test_schema($schema,'type="tns:testType"',(object)array("int1"=>1.1,"int2"=>2.2,"int3"=>3.3), "rpc", "literal", 'elementFormDefault="unqualified"');
echo "ok";
?>