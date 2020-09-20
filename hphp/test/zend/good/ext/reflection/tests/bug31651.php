<?hh

class Test
{
    public $a = darray['a' => 1];
}
<<__EntryPoint>> function main(): void {
$ref = new ReflectionClass('Test');

print_r($ref->getDefaultProperties());
}
