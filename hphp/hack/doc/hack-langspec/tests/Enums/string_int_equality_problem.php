<?hh // strict

namespace NS_string_int_equality_problem;

/* *** mixed enums no longer allowed

enum E5: mixed {
  Ready = 'R';
  Started = 0;
}

function main(E5 $p4): void {
//  var_dump('R' == 0);	// Surprise, this is True!
//  var_dump('R' === 0);	// This is False

// Apparently, this is a PHP "feature". The engine tries to convert the 'R' to int and fails, but continues
// by substituting zero instead!

  echo "++++++\n";
  var_dump($p4);	// 'R'
  echo $p4 . " == " . E5::Started . " is " . (($p4 == E5::Started) ? "True\n" : "False\n");	// True **
  echo $p4 . " === " . E5::Started . " is " . (($p4 === E5::Started) ? "True\n" : "False\n");	// False
  echo $p4 . " == " . E5::Ready . " is " . (($p4 == E5::Ready) ? "True\n" : "False\n");		// True
  echo $p4 . " === " . E5::Ready . " is " . (($p4 === E5::Ready) ? "True\n" : "False\n");	// True
  echo "++++++\n";

  switch ($p4) {
  case E5::Started:
    echo "Started: $p4\n";	// ??? comes here, but displays $p4 => 'R' (Ready)
    break;
  case E5::Ready:
    echo "Ready: $p4\n";
    break;
  }
}
*/

/* HH_FIXME[1002] call to main in strict*/
main(E5::Ready);
/* HH_FIXME[1002] call to main in strict*/
main(E5::Started);

