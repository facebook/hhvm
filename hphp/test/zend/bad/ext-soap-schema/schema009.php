<?php
include "test_schema.inc";
$schema = <<<EOF
	<simpleType name="testType">
		<list itemType="token"/>
	</simpleType>
EOF;
test_schema($schema,'type="tns:testType"',"one two");
echo "ok";
?>