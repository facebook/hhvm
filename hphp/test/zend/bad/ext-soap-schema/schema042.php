<?php
include "test_schema.inc";
$schema = <<<EOF
	<complexType name="testType">
		<simpleContent>
			<extension base="int">
				<attribute name="int" type="int"/>
			</extension>
		</simpleContent>
	</complexType>
EOF;
test_schema($schema,'type="tns:testType"',(object)array("_"=>123.5,"int"=>123.5));
echo "ok";
?>