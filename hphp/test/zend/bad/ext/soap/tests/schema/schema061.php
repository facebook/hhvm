<?php
include "test_schema.inc";
$schema = <<<EOF
	<complexType name="testType">
		<complexContent>
			<restriction base="enc12:Array" xmlns:enc12="http://www.w3.org/2003/05/soap-encoding">
				<all>
					<element name="x_item" type="int" maxOccurs="unbounded"/>
		    </all>
  	    <attribute ref="enc12:arraySize" wsdl:arraySize="* 1"/>
    	</restriction>
    </complexContent>
	</complexType>
EOF;
test_schema($schema,'type="tns:testType"',array(array(123),array(123.5)),'rpc','literal');
echo "ok";
?>