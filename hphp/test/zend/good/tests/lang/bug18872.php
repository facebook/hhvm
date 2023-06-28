<?hh
class FooBar {
    const BIFF = 3;
}

function foo($biff = FooBar::BIFF) :mixed{
    echo $biff . "\n";
}
<<__EntryPoint>> function main(): void {
foo();
foo();
}
