<?hh <<__EntryPoint>> function main(): void {
$strings = array('into', 'info', 'inf', 'infinity', 'infin', 'inflammable');
foreach ($strings as $v) {
    echo ($v+0)."\n";
}
}
