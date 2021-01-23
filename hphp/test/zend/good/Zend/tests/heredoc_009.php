<?hh

<<__EntryPoint>>
function entrypoint_heredoc_009(): void {

  require_once 'nowdoc.inc';
  try {
    print <<<ENDOFHEREDOC
ENDOFHEREDOC    ;
    ENDOFHEREDOC;
ENDOFHEREDOC    
    ENDOFHEREDOC
$ENDOFHEREDOC;

ENDOFHEREDOC;
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }

  try {
    $x = <<<ENDOFHEREDOC
ENDOFHEREDOC    ;
    ENDOFHEREDOC;
ENDOFHEREDOC    
    ENDOFHEREDOC
$ENDOFHEREDOC;

ENDOFHEREDOC;
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }

  try {
    print "{$x}";
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
}
