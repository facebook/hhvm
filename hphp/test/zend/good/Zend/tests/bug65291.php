<?hh

trait TestTrait
{
    public static function testStaticFunction()
    {
        return __CLASS__;
    }
}
class Tester
{
    use TestTrait;
}
<<__EntryPoint>> function main(): void {
$foo = Tester::testStaticFunction();
get_defined_constants();
get_defined_constants(true);

echo $foo;
}
