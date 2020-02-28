<?hh


<<__EntryPoint>>
function main_error_depth() {
$array = varray[];
for ($i=0; $i<550; $i++) {
    $array = varray[$array];
}

var_dump(json_encode($array, 0, 551));
switch (json_last_error()) {
    case JSON_ERROR_NONE:
        echo 'OK'.PHP_EOL;
    break;
    case JSON_ERROR_DEPTH:
        echo 'ERROR'.PHP_EOL;
    break;
}

var_dump(json_encode($array, 0, 540));
switch (json_last_error()) {
    case JSON_ERROR_NONE:
        echo 'OK'.PHP_EOL;
    break;
    case JSON_ERROR_DEPTH:
        echo 'ERROR'.PHP_EOL;
    break;
}
}
