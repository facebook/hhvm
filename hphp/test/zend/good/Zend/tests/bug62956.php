<?hh
class Base
{
    private function test()
    {}
}

class Extension extends Base
{
    private function test($arg)
    {}
}
<<__EntryPoint>> function main(): void {
echo "==DONE==";
}
