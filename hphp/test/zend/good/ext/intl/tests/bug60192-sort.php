<?hh

class Collator2 extends Collator{
    public function __construct() {
        // ommitting parent::__construct($someLocale);
    }
}
<<__EntryPoint>> function main(): void {
$c = new Collator2();
$a = vec['a', 'b'];
$c->sort(inout $a);
}
