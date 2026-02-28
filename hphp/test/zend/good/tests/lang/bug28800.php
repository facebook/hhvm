<?hh <<__EntryPoint>> function main(): void {
$strings = vec['into', 'info', 'inf', 'infinity', 'infin', 'inflammable'];
foreach ($strings as $v) {
    echo (HH\Lib\Legacy_FIXME\cast_for_arithmetic($v)+0)."\n";
}
}
