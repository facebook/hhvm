<?hh

class test {
    static function show_static() {
        echo "static\n";
    }
    function show_method() {
        echo "method\n";
    }
}

<<__EntryPoint>> function main(): void {
test::show_static();

$t = new test;
$t->show_method();
}
