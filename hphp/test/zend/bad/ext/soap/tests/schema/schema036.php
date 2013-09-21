<?php
include "test_schema.inc";
$schema = <<<EOF
	<complexType name="testType">
		<sequence>
			<element name="int" type="int"/>
			<element name="testType2">
				<complexType>
					<sequence>
						<element name="int" type="int"/>
					</sequence>
				</complexType>
			</element>
		</sequence>
	</complexType>
EOF;
test_schema($schema,'type="tns:testType"',(object)array("int"=>123.5,"testType2"=>array("int"=>123.5)));
echo "ok";
?>