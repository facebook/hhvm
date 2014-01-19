<?php
include "test_schema.inc";
$schema = <<<EOF
	<element name="testElement" type="tns:testType"/>
	<complexType name="testType">
		<complexContent>
			<restriction base="SOAP-ENC:Array">
  	    <attribute ref="SOAP-ENC:arrayType" wsdl:arrayType="int[]"/>
    	</restriction>
    </complexContent>
	</complexType>
EOF;
test_schema($schema,'element="tns:testElement"',array(123,123.5),'document','literal');
echo "ok";
?>