<?hh

const SCRIPT_NAME = __DIR__.'/command_line_arguments.inc';


<<__EntryPoint>>
function main_command_line_arguments() :mixed{
$cmd = HH\__internal\hhvm_binary().' --php -n -d foo=bar '.SCRIPT_NAME;
var_dump($cmd);
$return_var = -1;
system($cmd, inout $return_var);

$cmd = HH\__internal\hhvm_binary().' --php -n < '.SCRIPT_NAME;
var_dump($cmd);
system($cmd, inout $return_var);

$cmd = HH\__internal\hhvm_binary().' --php -n --define date.timezone=America/New_York -r "var_dump((string) (new DateTime())->format(\'e\'));"';
var_dump($cmd);
system($cmd, inout $return_var);

$cmd = HH\__internal\hhvm_binary().' --php -n --define date.timezone=America/Los_Angeles -r "var_dump((string) (new DateTime())->format(\'e\'));"';
var_dump($cmd);
system($cmd, inout $return_var);
}
