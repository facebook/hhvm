<?hh

class test {
    const val = 1;
}

function foo($v = test::val) {
    var_dump($v);
}

function bar($b = NoSuchClass::val) {
    var_dump($b);
}
<<__EntryPoint>> function main(): void {
foo();
foo(5);

bar(10);
bar();

echo "Done\n";
}
