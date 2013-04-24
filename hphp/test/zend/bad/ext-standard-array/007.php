<?php
$a = array(1,"big"=>2,3,6,3,5,3,3,3,3,3,3,3,3,3,3);
$b = array(2,2,3,3,3,3,3,3,3,3,3,3,3,3,3);
$c = array(-1,1);
echo '$a='.var_export($a,TRUE).";\n";
echo '$b='.var_export($b,TRUE).";\n";
echo '$c='.var_export($c,TRUE).";\n";
var_dump(array_diff($a,$b,$c));
var_dump(array_diff_assoc($a,$b,$c));
$a = array(
'a'=>2,
'b'=>'some',
'c'=>'done',
'z'=>'foo',
'f'=>5,
'fan'=>'fen',
7=>18,
9=>25,
11=>42,
12=>42,
45=>42,
73=>'foo',
95=>'some',
'som3'=>'some',
'want'=>'wanna');
$b = array(
'a'=>7,
7=>18,
9=>13,
11=>42,
45=>46,
'som3'=>'some',
'foo'=>'some',
'goo'=>'foo',
'f'=>5,
'z'=>'equal'
);
$c = array(
73=>'foo',
95=>'some');
echo '$a='.var_export($a,TRUE).";\n";
echo '$b='.var_export($b,TRUE).";\n";
echo '$c='.var_export($c,TRUE).";\n";
echo "Results:\n\n";
var_dump(array_diff($a,$b,$c));
var_dump(array_diff_assoc($a,$b,$c));

echo "-=-=-=-=-=-=-=-=- New functionality from 5.0.0 -=-=-=-=-=-=-=-\n";
error_reporting(E_ALL);
class cr {
	private $priv_member;
	public  $public_member;
	function cr($val) {
		$this->priv_member = $val;
		$this->public_member = $val;
	}
	static function comp_func_cr($a, $b) {
		if ($a->priv_member === $b->priv_member) return 0;
		return ($a->priv_member > $b->priv_member)? 1:-1;
	}
}

function comp_func($a, $b) {
	if ($a === $b) return 0;
	return ($a > $b)? 1:-1;

}

function comp_func_cr($a, $b) {
	if ($a->public_member === $b->public_member) return 0;
	return ($a->public_member > $b->public_member)? 1:-1;
}


/*
$a = array(1,"big"=>2,3,6,3,5,3,3,3,3,3,3,3,3,3,3);
$b = array(2,2,3,3,3,3,3,3,3,3,3,3,3,3,3);
$c = array(-1,1);
echo '$a='.var_export($a,TRUE).";\n";
echo '$b='.var_export($b,TRUE).";\n";
echo '$c='.var_export($c,TRUE).";\n";
var_dump(array_diff($a,$b,$c));
var_dump(array_diff_assoc($a,$b,$c));
var_dump(array_udiff($a, $b, $c, "comp_func"));
var_dump(array_diff_uassoc($a,$b,$c, "comp_func"));
*/

/*
 $a = array(new cr(9),new cr(12),new cr(23),new cr(4),new cr(-15),);
 $b = array(new cr(9),new cr(22),new cr( 3),new cr(4),new cr(-15),);
 var_dump(array_udiff($a, $b, "comp_func_cr"));
*/
$a = array("0.1" => new cr(9), "0.5" => new cr(12), 0 => new cr(23), 1=> new cr(4), 2 => new cr(-15),);
$b = array("0.2" => new cr(9), "0.5" => new cr(22), 0 => new cr( 3), 1=> new cr(4), 2 => new cr(-15),);

echo '$a='.var_export($a,TRUE).";\n";
echo '$b='.var_export($b,TRUE).";\n";
echo 'var_dump(array_udiff_uassoc($a, $b, "comp_func_cr", "comp_func"));'."\n";
var_dump(array_udiff_uassoc($a, $b, "comp_func_cr", "comp_func"));


echo '$a='.var_export($a,TRUE).";\n";
echo '$b='.var_export($b,TRUE).";\n";
echo 'var_dump(array_udiff_uassoc($a, $b, array("cr", "comp_func_cr"), "comp_func"));'."\n";
var_dump(array_udiff_uassoc($a, $b, array("cr", "comp_func_cr"), "comp_func"));


echo '$a='.var_export($a,TRUE).";\n";
echo '$b='.var_export($b,TRUE).";\n";
echo 'var_dump(array_udiff($a, $b, "comp_func_cr"));'."\n";
var_dump(array_udiff($a, $b, "comp_func_cr"));


echo '$a='.var_export($a,TRUE).";\n";
echo '$b='.var_export($b,TRUE).";\n";
echo 'var_dump(array_udiff_assoc($a, $b, "comp_func_cr"));'."\n";
var_dump(array_udiff_assoc($a, $b, "comp_func_cr"));

?>