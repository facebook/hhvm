<?hh <<__EntryPoint>> function main(): void {
error_reporting(E_ALL);

var_dump(vec[1, 2, 3, 4,][3]);
try { var_dump(vec[1, 2, 3, 4]['foo']); } catch (Exception $e) { echo $e->getMessage()."\n"; }
var_dump(vec[vec[1,2,3], vec[4, 5, 6]][1][2]);

foreach (vec[vec[1, 2, 3]][0] as $var) {
     echo $var;
}
}
