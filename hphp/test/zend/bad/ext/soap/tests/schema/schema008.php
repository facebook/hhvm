<?php
include "test_schema.inc";
$schema = <<<EOF
<element name="testElement">
	<simpleType>
		<restriction>
			<simpleType name="testType2">
		    <restriction base="xsd:int"/>
	    </simpleType>
	  </restriction>
	</simpleType>
</element>
EOF;
test_schema($schema,'element="tns:testElement"',123.5);
echo "ok";
?>