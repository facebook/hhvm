<?hh

class Collator2 extends Collator{
    public function __construct() {
        // ommitting parent::__construct($someLocale);
    }
}
<<__EntryPoint>> function main(): void {
$c = new Collator2();
$a = varray['a', 'b'];
$c->sortWithSortKeys(inout $a);
}
