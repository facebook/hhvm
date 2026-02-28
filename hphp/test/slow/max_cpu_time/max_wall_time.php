<?hh

function sleep2() :mixed{ sleep(2); }

<<__EntryPoint>>
function main_max_wall_time() :mixed{
ini_set('hhvm.max_wall_time', 1);
ini_set('hhvm.max_cpu_time', 1);
var_dump(ini_get('hhvm.max_wall_time'));
sleep2();
echo "done\n";
}
