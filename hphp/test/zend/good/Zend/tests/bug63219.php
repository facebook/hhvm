<?hh
trait TFoo {
    public function fooMethod(){}
}

class C {
    use TFoo {
        Typo::fooMethod as tf;
    }
}
<<__EntryPoint>> function main(): void {
echo "okey";
}
