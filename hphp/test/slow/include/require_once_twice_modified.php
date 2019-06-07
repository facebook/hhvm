<?hh


<<__EntryPoint>>
function main_require_once_twice_modified() {
$file = tempnam('/tmp', 'require_once_twice.inc');

file_put_contents($file, "<?hh\n echo 'Hello world!\n';\n"
                         ."function f() { return 3; }\n");
require_once($file);
var_dump(f());

sleep(2);
touch($file);
require_once($file);

file_put_contents($file, "<?hh\n echo 'Shouldn't be visible\n';\n"
                         ."function f() { return 5; }\n");
require_once($file);
var_dump(f());

unlink($file);
}
