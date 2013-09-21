<?php
include "test_schema.inc";
$schema = <<<EOF
	<complexType name="testType">
		<attribute name="int1" type="int"/>
		<attribute name="int2" type="int" form="qualified"/>
		<attribute name="int3" type="int" form="unqualified"/>
	</complexType>
EOF;

test_schema($schema,'type="tns:testType"',(object)array("int1"=>1.1,"int2"=>2.2,"int3"=>3.3), "rpc", "encoded", 'attributeFormDefault="qualified"');
echo "ok";
?>