<?hh

function foo(bool $x = true, bool $y = false) :mixed{
    var_dump($x, $y);
}


<<__EntryPoint>> function main(): void {
foo();
}
