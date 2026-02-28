<?hh


<<__EntryPoint>>
function main_minmax_splat() :mixed{
$arr = vec[2, 3];
var_dump(min(1, ...$arr));
var_dump(max(1, ...$arr));
}
