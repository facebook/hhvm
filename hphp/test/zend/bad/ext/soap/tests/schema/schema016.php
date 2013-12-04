<?php
include "test_schema.inc";
$schema = <<<EOF
	<simpleType name="testType">
		<union>
			<simpleType>
				<restriction base="string"/>
			</simpleType>
			<simpleType>
				<restriction base="int"/>
			</simpleType>
			<simpleType>
				<restriction base="float"/>
			</simpleType>
		</union>
	</simpleType>
EOF;
test_schema($schema,'type="tns:testType"',123.5);
echo "ok";
?>