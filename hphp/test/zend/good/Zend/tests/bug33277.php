<?hh
class foo {
        private function bar() :mixed{
                echo "private!\n";
        }
}

class fooson extends foo {
        function barson() :mixed{
                $this->bar();
        }
}

class foo2son extends fooson {

        function bar() :mixed{
                echo "public!\n";
        }
}
<<__EntryPoint>> function main(): void {
$b = new foo2son();
$b->barson();
}
