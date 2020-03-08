<?hh // strict

namespace NS_switch;

function main(): void {
  echo "================= empty body ====================\n";

// Can I have an empty body?

  $v = 10;
  switch ($v) {		// yes
  }
/*
  echo "================= body containing only empty statements ====================\n";

// presumably, body can be one or more empty statements

  $v = 10;
  switch ($v) {
    ;		// unlike PHP, not even 1 is permitted
    //;
    //;
  }
*/

  echo "================= default case only ====================\n";

  $v = 10;
  switch ($v) {
  default:
    echo "default case: \$v is $v\n";
  }

  echo "================= general switch ====================\n";

  $v = 10;
  switch ($v) {
  default:
    // FALLTHROUGH
    echo "default case: \$v is $v\n";
    break;		// break ends "group" of default statements
  case 10:
    // FALLTHROUGH
    // FALLTHROUGH
    echo "case 10\n";	// no break, so control drops into next label's "group"
    // FALLTHROUGH
  case 30:
    // FALLTHROUGH
    echo "case 30\n";	// no break, but then none is really needed either
    //		return;
  case 20:
    echo "case 20\n";
    break; 		// break ends "group" of case 20 statements
    // FALLTHROUGH
  }

  echo "================= duplicate case values ====================\n";

// Check duplicate case values: allowed; choses lexically first one

  $v = 30;
  switch ($v) {
  case 30:
    echo "case 30-2\n";
    break;
  default:
    echo "default case: \$v is $v\n";
    break;
  case 30:
    echo "case 30-1\n";
    break;
  }

  echo "================= multiple case candidates ====================\n";

// chooses first match with equal value, 30 matches 30.0 before 30

  $v = 30;
  switch ($v) {
  case 30.0:	// <===== this case matches with 30
    echo "case 30.0\n";
    break;
  default:
    echo "default case: \$v is $v\n";
    break;
  case 30:		// <===== rather than this case matching with 30
    echo "case 30\n";
    break;
  }

/* Unlike PHP, the case/default label terminator must be a :

  echo "================= terminating case/default labels with ; ====================\n";

// ; is allowed in place of : at end of case/default label; can mix-n-match

  $v = 10;
  switch ($v) {
  case 10;		// <================ ;
    echo "case 10\n";
    break;
  case 20:		// <================ :
    echo "case 20\n";
    break;
  default;		// <================ ;
    echo "default case: \$v is $v\n";
    break;
  }
*/

  echo "================= strings for label values ====================\n";

// use  strings for label values 

  $v = "white";	// note the lower-case spelling
  switch ($v) {
  case "White":
    echo "case White\n";
    break;
  case "Red":
    echo "case Red\n";
    break;
  default:
    echo "default case: \$v is $v\n";
    break;
  }

  echo "================= Booleans for label values ====================\n";

// use Booleans for label values 

  $v = true;
  switch ($v) {
  case false:
    echo "case false\n";
    break;
  case true:
   echo "case true\n";
   break;
  default:
    echo "default case: \$v is $v\n";
    break;
  }

  echo "================= expressions for label values ====================\n";

// use expressions for label values 

  $v = 22;
  $a = 1;
  $b = 12;
  switch ($v) {
  case 10 + $b:
    echo "case 10 + $b\n";
    break;
  case (int)($v < $a):		// cast needed; otherwise, label expression type incompatible
 				// with switch expression type
    echo "case $v < $a\n";
    break;
  default:
    echo "default case: \$v is $v\n";
    break;
  }
}

/* HH_FIXME[1002] call to main in strict*/
main();
