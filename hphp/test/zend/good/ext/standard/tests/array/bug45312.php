<?hh
class cr {
    private $priv_member;
    function __construct($val) {
        $this->priv_member = $val;
    }
    <<__DynamicallyCallable>>static function comp_func_cr($a, $b) {
        if ($a->priv_member === $b->priv_member) return 0;
        return ($a->priv_member > $b->priv_member) ? 1 : -1;
    }
    <<__DynamicallyCallable>> static function comp_func_cr2($a, $b) {
        echo ".";
        if ($a->priv_member === $b->priv_member) return 0;
        return ($a->priv_member < $b->priv_member) ? 1 : -1;
    }
    function dump() {
        echo $this->priv_member . "\n";
    }
}
<<__EntryPoint>> function main(): void {
$a = darray["0.1" => new cr(9), "0.5" => new cr(12), 0 => new cr(23), 1 => new cr(4), 2 => new cr(-15),];
$b = darray["0.2" => new cr(9), "0.5" => new cr(22), 0 => new cr(3), 1 => new cr(4), 2 => new cr(-15),];
$result = array_udiff_assoc($a, $b, varray["cr", "comp_func_cr"]);
foreach($result as $val) {
    $val->dump();
}
$result = array_udiff_assoc($a, $b, varray["cr", "comp_func_cr2"]);
foreach($result as $val) {
    $val->dump();
}
}
