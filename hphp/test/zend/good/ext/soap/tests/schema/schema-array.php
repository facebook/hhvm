<?hh
<<__EntryPoint>> function main(): void {
include "test_schema.inc";
$schema = <<<EOF
  <complexType name="testType">
    <sequence>
      <element name="testItem" minOccurs="0" maxOccurs="unlimited"/>
    </sequence>
  </complexType>
EOF;

test_schema($schema, 'testType', darray['testItem' => varray[17, 'foo']]);
test_schema($schema, 'testType', darray['testItem' => vec[34, 'foo']]);
test_schema($schema, 'testType', dict['testItem' => varray[51, 'foo']]);
test_schema($schema, 'testType', dict['testItem' => vec[68, 'foo']]);
}
