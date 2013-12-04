<?php
include "test_schema.inc";
$schema = <<<EOF
	<simpleType name="testType">
		<union memberTypes="string int float"/>
	</simpleType>
EOF;
test_schema($schema,'type="tns:testType"',123.5);
echo "ok";
?>