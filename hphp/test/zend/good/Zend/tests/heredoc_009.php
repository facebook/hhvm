<?hh

<<__EntryPoint>>
function entrypoint_heredoc_009(): void {

  require_once 'nowdoc.inc';

  print <<<ENDOFHEREDOC
ENDOFHEREDOC    ;
    ENDOFHEREDOC;
ENDOFHEREDOC    
    ENDOFHEREDOC
$ENDOFHEREDOC;

ENDOFHEREDOC;

  $x = <<<ENDOFHEREDOC
ENDOFHEREDOC    ;
    ENDOFHEREDOC;
ENDOFHEREDOC    
    ENDOFHEREDOC
$ENDOFHEREDOC;

ENDOFHEREDOC;

  print "{$x}";
}
