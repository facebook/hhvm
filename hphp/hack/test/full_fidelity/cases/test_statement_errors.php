<?hh
function bar() : void {
  case 123: ; // error, outside switch
  default: ; // error, outside switch
  switch ($x) { case 123: ; default: break;} // no error
  break; // error
  while($a) {
    if ($b) break; // no error
    if ($c) continue; // no error
    $x = function($b) { break; }; // error
  }
  try {} catch {} // no error
  try {} finally {} // no error
  try {} // error
  switch ($x) { } // no error
  switch ($x) { foo(); } // error
  switch ($x) { foo(); foo(); } // error
}
