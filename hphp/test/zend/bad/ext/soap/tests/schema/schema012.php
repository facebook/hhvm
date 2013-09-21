<?php
include "test_schema.inc";
$schema = <<<EOF
	<simpleType name="testType">
		<list>
			<simpleType>
				<restriction base="int"/>
			</simpleType>
		</list>
	</simpleType>
EOF;
test_schema($schema,'type="tns:testType"',array(123,456.7));
echo "ok";
?>