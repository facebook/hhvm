<?hh // strict

namespace NS_if;

function processTransaction(): void { echo "Inside processTransaction\n"; }
function postMessage(): void { echo "Inside postMessage\n"; }

class Name {
  private string $firstName = "xxx";
  private string $lastName = "yyy";
}

function main(): void {
// an ordinary if having 2 actions on true and none on false

  $count = 5;
  if ($count > 0) {
    processTransaction();
    postMessage();
  }

// despite the indenting that suggests the truepath has 2 statements, in the
// absence of braces, the truepath is the first statement only. The second statement
// is always executed.

  if (0)
    echo "Line 1\n";
    echo "Line 2\n";	// always executed

// use if with all scalar types + array

  $colors = array("red", "white", "blue");
  $scalarValueList = array(10, -100, 0, 1.234, 0.0, true, false, null, 'xx', "", $colors);

  foreach ($scalarValueList as $e) {
    if ($e) {
      echo ">" . (string)$e . "< is true\t"; var_dump($e);
    } else {
      echo ">" . (string)$e . "< is false\t"; var_dump($e);
    }
  }

// use if with an instance of a class

  $aName = new Name();
  var_dump($aName);

  if ($aName) {
    echo ">\$aName< is true\n";
  } else {
    echo ">\$aName< is false\n";
  }

// show that when elses are nested, an else matches the lexically nearest preceding if that is allowed by the syntax

  if (1)
    echo "Path 1\n";
    if (0)
      echo "Path 2\n";
  else 	// this else does NOT go with the outer if
      echo "Path 3\n";

  if (1) {
    echo "Path 1\n";
    if (0)
      echo "Path 2\n";
  } else 	// this else does go with the outer if
    echo "Path 3\n";

// test elseif

  $a = 10;
  if ($a < 0)
    ; // ...
  elseif ($a == 0)
    ; // ...
  elseif ($a < 10)
    ; // ...
  else
    ; // ...

/*
// alternate syntax is not supported

  if ($a < 0)  : // ...
    ++$a; // ...
  endif  ; // ...

  if ($a < 0)  : // ...
    ++$a; // ...
  else :
    --$a; // ...
  endif  ; // ...

  if ($a < 0):
    ; // ...
  elseif ($a == 0)  :
    ; // ...
  elseif ($a < 10):
    ; // ...
  else:
    ; // ...
  endif;
*/
}

/* HH_FIXME[1002] call to main in strict*/
main();
