<?hh

class bar  {
    public function __call($a, $b) {
        print "__call:\n";
        var_dump($a);
    }
    public function test() {
        self::ABC();
        bar::ABC();
        BAR::xyz();
        BAR::www();
        self::y();
        self::y();
    }
    static function x() {
        print "ok\n";
    }
}
<<__EntryPoint>> function main(): void {
$x = new bar;

$x->test();

call_user_func(varray['BAR','x']);
call_user_func('self::y');
}
