<?hh

class CompletionValues {
  public static mixed $values = null;
}

class InvalidCompletion {}

<<__EntryPoint>>
function main(): void {
  $throws = count(HH\global_get('argv')) > 1;
  if ($throws) {
    set_error_handler(($errno, $message, ...$_) ==> {
      throw new Exception($message);
    });
  }
  readline_completion_function(($text, $start, $end) ==> {
    CompletionValues::$values = $throws
      ? vec[new InvalidCompletion()]
      : vec[$text.'llo'];
    return CompletionValues::$values;
  });
  try {
    $line = readline('input> ');
    echo "completed: ", trim($line), "\n";
  } catch (Exception $e) {
    echo "\ncaught: ", $e->getMessage(), "\n";
  }
  debug_zval_dump(CompletionValues::$values);
}
