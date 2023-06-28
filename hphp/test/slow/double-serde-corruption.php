<?hh


<<__EntryPoint>>
function main_double_serde_corruption() :mixed{
$val = -9.2233720368547758e18;
var_dump($val == unserialize(serialize($val)));
}
