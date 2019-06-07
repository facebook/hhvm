<?hh


<<__EntryPoint>>
function main_fork_log() {
var_dump($g);
if (!pcntl_fork()) {
  var_dump($g);
}
}
