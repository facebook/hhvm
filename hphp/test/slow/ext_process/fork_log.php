<?hh


<<__EntryPoint>>
function main_fork_log() :mixed{
  try {
    var_dump($g);
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
  if (!pcntl_fork()) {
    try {
      var_dump($g);
    } catch (UndefinedVariableException $e) {
      var_dump($e->getMessage());
    }
  }
}
