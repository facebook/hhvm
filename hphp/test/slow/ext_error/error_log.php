<?hh


<<__EntryPoint>>
function main_error_log() {
$log_string = 'hello world';
$filename = sys_get_temp_dir().'/'.'errorlog_test';

error_log($log_string, 3, $filename);
$f = fopen($filename, 'r');
$content = fgets($f);
var_dump($content);
fclose($f);

// test that the logging is appending without newlines
error_log($log_string, 3, $filename);
$f = fopen($filename, 'r');
$content = fgets($f);
var_dump($content);
fclose($f);

unlink($filename);
}
