<?hh

class test {
    static function show_static() :mixed{
        echo "static\n";
    }
    function show_method() :mixed{
        echo "method\n";
    }
}

<<__EntryPoint>> function main(): void {
test::show_static();

$t = new test;
$t->show_method();
}
