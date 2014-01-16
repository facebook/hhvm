<?php
include "test_schema.inc";
$schema = <<<EOF
	<simpleType name="testType">
		<restriction base="tns:testType2"/>
	</simpleType>
EOF;
test_schema($schema,'type="tns:testType"',123.5);
echo "ok";
?>