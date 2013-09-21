<?php
include "test_schema.inc";
$schema = <<<EOF
	<complexType name="testType">
		<complexContent>
			<restriction base="SOAP-ENC:Array">
				<all>
					<element name="x_item" type="int" maxOccurs="unbounded"/>
		    </all>
    	</restriction>
    </complexContent>
	</complexType>
EOF;
test_schema($schema,'type="tns:testType"',array(123,123.5),'rpc','literal');
echo "ok";
?>