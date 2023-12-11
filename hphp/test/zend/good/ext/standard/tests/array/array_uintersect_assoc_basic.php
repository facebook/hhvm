<?hh
/*
* proto array array_uintersect_assoc ( array $array1, array $array2 [, array $ ..., callback $data_compare_func] )
* Function is implemented in ext/standard/array.c
*/
class cr {
    private $priv_member;
    function __construct($val) {
        $this->priv_member = $val;
    }
    <<__DynamicallyCallable>> static function comp_func_cr($a, $b) :mixed{
        if ($a->priv_member === $b->priv_member) return 0;
        return ($a->priv_member > $b->priv_member) ? 1 : -1;
    }
}
<<__EntryPoint>> function main(): void {
$a = dict["0.1" => new cr(9), "0.5" => new cr(12), 0 => new cr(23), 1 => new cr(4), 2 => new cr(-15),];
$b = dict["0.2" => new cr(9), "0.5" => new cr(22), 0 => new cr(3), 1 => new cr(4), 2 => new cr(-15),];
$result = array_uintersect_assoc($a, $b, vec["cr", "comp_func_cr"]);
var_dump($result);
}
