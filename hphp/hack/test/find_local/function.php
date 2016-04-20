<?hh
                                    //  2
function test($x) {                 //  3 Should match
  $x = 3; // Looking for this $x.   //  4 Should match
  g($x) + $x + h("\$x = $x");       //  5 First three should match
  $lambda1 = $x ==> $x + 1;         //  6 Should not match
  $lambda2 = $a ==> $x + $a;        //  7 Should match
  $lambda3 = function($x) {         //  8 Should not match
    return $x + 1; };               //  9 Should not match
  $lambda4 = function($b) use($x) { // 10 Should match
    return $x + $b; };              // 11 Should match
  $lambda5 = function($x) use($x) { // 12 Second should match
    return $x; };                   // 13 Should not match
  if (f($x)) g($x); else h($x);     // 14 Three should match
  do { g($x); } while (f($x));      // 15 Two should match
  for ($x = 1; $x < 3; $x = 2 * $x) // 16 Four should match
    while (f(shape('x' => $x))      // 17 Should match
      foreach($x as $x)             // 18 Two should match
        foreach($x as $x => $x) ;   // 19 Three should match
  try { f(clone $x); }              // 20 Should match
  catch (F $x) { g($x); }           // 21 Should not match
  catch (G $e) { g($x); }           // 22 Should match
  switch (f($x)) {                  // 23 Should match
   case $x: g($x); break; }         // 24 Two should match
  echo  $x, (string)$x;             // 25 Two should match
  return <div>{$x}</div>;    }      // 26 Should match
