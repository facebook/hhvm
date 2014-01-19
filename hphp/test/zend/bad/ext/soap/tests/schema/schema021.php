<?php
include "test_schema.inc";
$schema = <<<EOF
	<simpleType name="testType">
		<list>
			<simpleType>
				<union memberTypes="int float str"/>
			</simpleType>
		</list>
	</simpleType>
EOF;
test_schema($schema,'type="tns:testType"',"123 123.5 456.7 str");
echo "ok";
?>