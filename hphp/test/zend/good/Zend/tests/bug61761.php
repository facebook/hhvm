<?hh

class A
{
        private static function test($a) :mixed{ }
}

class B extends A
{
        private static function test($a, $b) :mixed{ }
}
<<__EntryPoint>> function main(): void {
echo "==DONE==";
}
