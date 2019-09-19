<?hh

function fatal_handler() {
    chmod($GLOBALS['file'], 0600);
    @unlink($GLOBALS['file']);
}


<<__EntryPoint>>
function main_open_rc_check() {
register_shutdown_function(fun('fatal_handler'));

$file = tempnam(sys_get_temp_dir(), 'cannotopen');
$data = "//Nothing";
$GLOBALS['file'] = $file;

if (file_put_contents($GLOBALS['file'], $data) !== false && chmod($GLOBALS['file'], 0000)) {
    require $GLOBALS['file'];
}
}
