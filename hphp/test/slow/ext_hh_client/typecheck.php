<?hh

$clean = \HH\Client\typecheck(__DIR__.'/hh_client_clean');
var_dump($clean);
echo json_encode($clean);
echo "\n";

$error = \HH\Client\typecheck(__DIR__.'/hh_client_error');
var_dump($error);
echo json_encode($error);
echo "\n";

$noclient = \HH\Client\typecheck(__DIR__.'/this_file_does_not_exist');
var_dump($noclient);
echo json_encode($noclient);
echo "\n";

$clean->triggerError(E_WARNING);
$error->triggerError(E_WARNING);
$error->triggerError(E_RECOVERABLE_ERROR);
