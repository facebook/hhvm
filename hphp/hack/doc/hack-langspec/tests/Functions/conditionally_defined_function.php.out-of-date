<?hh // strict

namespace NS_functions_conditionally-defined;

// Unconditionally defined function

function ucf1(): void {
  echo "Inside unconditionally defined function ucf1\n";
}

// Conditionally defined function

/*
// nested function definitions are not supported

function ucf2(): void {
  function cf2(): void {
    echo "Inside conditionally defined function cf2\n";
  }
}
*/

function main(): void {
  ucf1();		// can call ucf1 before its definition is seen

  ucf1();		// can call ucf1 after its definition is seen

  $flag = true;
/*
  cf1();	// Error; call to undefined function

  if ($flag) {
// nested/conditional function definitions are not supported
    function cf1(): void {
      echo "Inside conditionally defined function cf1\n";
    }
  }

  if ($flag) {
    cf1();	// can call cf1 now, as it's been defined
  } else {
    cf1();	// Error; call to undefined function
  }
*/
//  cf2();		// Error; call to undefined function
//  ucf2();		// cf2 now exists
//  cf2();		// Ok
}

/* HH_FIXME[1002] call to main in strict*/
main();
