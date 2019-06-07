<?hh
    error_reporting(E_ALL & !E_STRICT);

    class A {
        static function hello() {
            echo "Hello World\n";
        }
    }
    $y = ['hello'];
    A::$y[0]();
<<__EntryPoint>> function main() {
echo "===DONE===\n";
}
