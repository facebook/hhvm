<?hh
<<__EntryPoint>> function main(): void {
include "test_schema.inc";
$schema = <<<EOF
    <complexType name="testType">
        <attribute name="str" type="string"/>
        <attribute name="int" type="int" fixed="5"/>
    </complexType>
EOF;
test_schema($schema,'testType', dict["str"=>"str","int"=>5]);
test_schema($schema,'testType', dict["str"=>"str","int"=>4]);
}
