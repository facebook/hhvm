<?hh

class A
{
        private static function test($a) { }
}

class B extends A
{
        private static function test($a, $b) { }
}
<<__EntryPoint>> function main(): void {
echo "==DONE==";
}
