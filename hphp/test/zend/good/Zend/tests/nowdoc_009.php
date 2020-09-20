<?hh

<<__EntryPoint>>
function entrypoint_nowdoc_009(): void {

  require_once 'nowdoc.inc';

  print <<<'ENDOFNOWDOC'
ENDOFNOWDOC    ;
    ENDOFNOWDOC;
ENDOFNOWDOC    
    ENDOFNOWDOC
$ENDOFNOWDOC;

ENDOFNOWDOC;

  $x = <<<'ENDOFNOWDOC'
ENDOFNOWDOC    ;
    ENDOFNOWDOC;
ENDOFNOWDOC    
    ENDOFNOWDOC
$ENDOFNOWDOC;

ENDOFNOWDOC;

  print "{$x}";
}
