<?hh

class C { public $a; }
<<__EntryPoint>> function main(): void {
$x = new C();
$x->a = function () use ($x) {
    $h = function () use ($x) {
    var_dump($x->a);
        return 1;
    };
    return $h();
};

var_dump(($x->a)());
}
