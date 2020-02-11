<?hh
include "test_schema.inc";
<<__EntryPoint>> function main(): void {
$schema = <<<EOF
    <complexType name="testType">
        <attribute name="str" type="string"/>
        <attribute name="int" type="int" fixed="5"/>
    </complexType>
EOF;
test_schema($schema,'type="tns:testType"', darray["str"=>"str","int"=>4]);
echo "ok";
}
