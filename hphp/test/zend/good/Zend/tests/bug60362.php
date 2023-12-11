<?hh <<__EntryPoint>> function main(): void {
$arr = dict['exists' => 'foz'];

if (isset($arr['exists']['non_existent'])) {
    echo "sub-key 'non_existent' is set: ";
    var_dump($arr['exists']['non_existent']);
} else {
    echo "sub-key 'non_existent' is not set.\n";
}
if (isset($arr['exists'][1])) {
    echo "sub-key 1 is set: ";
    var_dump($arr['exists'][1]);
} else {
    echo "sub-key 1 is not set.\n";
}

echo "-------------------\n";
if (isset($arr['exists']['non_existent']['sub_sub'])) {
    echo "sub-key 'sub_sub' is set: ";
    var_dump($arr['exists']['non_existent']['sub_sub']);
} else {
    echo "sub-sub-key 'sub_sub' is not set.\n";
}
if (isset($arr['exists'][1][0])) {
    echo "sub-sub-key 0 is set: ";
    var_dump($arr['exists'][1][0]);
} else {
    echo "sub-sub-key 0 is not set.\n";
}
echo "DONE";
}
