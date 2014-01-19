<?php
include "test_schema.inc";
$schema = <<<EOF
	<complexType name="testType2">
		<sequence>
			<element name="int" type="int"/>
		</sequence>
	</complexType>
	<complexType name="testType">
		<complexContent>
			<extension base="tns:testType2">
				<sequence>
					<element name="int2" type="int"/>
				</sequence>
			</extension>
		</complexContent>
	</complexType>
EOF;
class A {
  public $int = 1;
}

class B extends A {
  public $int2 = 2;
}


test_schema($schema,'type="tns:testType"',new B());
echo "ok";
?>