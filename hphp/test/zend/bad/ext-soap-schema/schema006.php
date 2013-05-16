<?php
include "test_schema.inc";
$schema = <<<EOF
	<simpleType name="testType">
		<restriction>
			<simpleType name="testType2">
		    <restriction base="xsd:int"/>
	    </simpleType>
	  </restriction>
	</simpleType>
	<element name="testElement" type="tns:testType"/>
EOF;
test_schema($schema,'element="tns:testElement"',123.5);
echo "ok";
?>