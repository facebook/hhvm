<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

<<__Memoize>>
function memo_fn(int $p, named int $zn, named int $an = 5): string {
    echo "memo_fn body\n";
    return "$p/$an/$zn";
}

class C {
    <<__Memoize>>
    public function meth(int $p, named int $an = 5): string {
        echo "meth body\n";
        return "$p/$an";
    }

    <<__MemoizeLSB>>
    public static function stat(named int $an): string {
        echo "stat body\n";
        return "$an";
    }
}

<<__EntryPoint>>
function main() {
    var_dump(memo_fn(1, zn=2));
    var_dump(memo_fn(1, zn=2));
    // Same key as above: the default fills $an in before the memo key is taken.
    var_dump(memo_fn(1, zn=2, an=5));
    var_dump(memo_fn(1, zn=3));

    $c = new C();
    var_dump($c->meth(1));
    var_dump($c->meth(1, an=5));
    var_dump($c->meth(1, an=6));

    var_dump(C::stat(an=1));
    var_dump(C::stat(an=1));
}
