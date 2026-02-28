<?hh
<<file: __EnableUnstableFeatures('named_parameters', 'named_parameters_use')>>

class C {
    public function __construct(named int $x, named int $y) {
        var_dump($x);
        var_dump($y);
    }
}

<<__EntryPoint>>
function main() {
  new C(x=1, y=2);
  new C(y=2, x=1);
}
