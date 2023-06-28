<?hh
interface foo {
    const foo = 'foobar';
    public function bar($x = foo):mixed;
}

class foobar implements foo {
    const foo = 'bar';
    public function bar($x = foo::foo) :mixed{
        var_dump($x);
    }
}

<<__EntryPoint>>
function main(): void {
  echo "Done.\n";
}
