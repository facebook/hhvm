<?hh
<<__EntryPoint>> function main(): void {
include "test_schema.inc";
$schema = <<<EOF
  <complexType name="testType">
    <attribute name="testAttr" type="int"/>
  </complexType>
EOF;

test_schema($schema, 'testType', dict['testAttr' => 17]);
test_schema($schema, 'testType', dict['testAttr' => 'foo']);
}
