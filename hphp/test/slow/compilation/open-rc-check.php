<?hh

function fatal_handler() {
    chmod(\HH\global_get('file'), 0600);
    @unlink(\HH\global_get('file'));
}


<<__EntryPoint>>
function main_open_rc_check() {
register_shutdown_function(fun('fatal_handler'));

$file = tempnam(sys_get_temp_dir(), 'cannotopen');
$data = "//Nothing";
\HH\global_set('file', $file);

if (file_put_contents(\HH\global_get('file'), $data) !== false && chmod(\HH\global_get('file'), 0000)) {
    require \HH\global_get('file');
}
}
