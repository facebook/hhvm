<?php
include "test_schema.inc";
$schema = <<<EOF
	<complexType name="testType2">
		<simpleContent>
			<extension base="int">
				<attribute name="int" type="int"/>
			</extension>
		</simpleContent>
	</complexType>
	<complexType name="testType">
		<complexContent>
			<restriction base="tns:testType2">
				<attribute name="int2" type="int"/>
			</restriction>
		</complexContent>
	</complexType>
EOF;
test_schema($schema,'type="tns:testType"',(object)array("_"=>123.5,"int"=>123.5,"int2"=>123.5));
echo "ok";
?>