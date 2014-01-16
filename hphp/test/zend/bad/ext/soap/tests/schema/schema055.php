<?php
include "test_schema.inc";
$schema = <<<EOF
	<complexType name="testType">
		<complexContent>
			<extension base="apache:Map" xmlns:apache="http://xml.apache.org/xml-soap">
    	</extension>
    </complexContent>
	</complexType>
EOF;
test_schema($schema,'type="testType"',array('a'=>123,'b'=>123.5));
echo "ok";
?>