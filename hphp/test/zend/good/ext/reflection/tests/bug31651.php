<?hh

class Test
{
    public $a = array('a' => 1);
}
<<__EntryPoint>> function main() {
$ref = new ReflectionClass('Test');

print_r($ref->getDefaultProperties());
}
