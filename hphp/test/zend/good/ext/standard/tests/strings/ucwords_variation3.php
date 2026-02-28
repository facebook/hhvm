<?hh
/* Prototype  : string ucwords ( string $str )
 * Description: Uppercase the first character of each word in a string
 * Source code: ext/standard/string.c
*/

/*
 * test ucwords() with different string prepared using single quote
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing ucwords() : usage variations ***\n";

// different strings containing regular chars and special chars
$str_array = vec[
  // multiple spaces
  'testing    ucwords',
  't e s t i n g   u c w o r d s ',

  // brackets in sentence
  'testing function(ucwords)',
  '(testing ( function (ucwords) )a )test',
  '(t)',
  ' ( t )t',

  // using quote chars in sentence
  '"testing",ucword,"test"',
  '"t""t",test, t',
  '\'t \'t\',test',
  
  // using other white spaces
  '\ttesting\ttesting\tucwords',
  'testing\rucwords testing ucwords',
  'testing\fucwords \f testing \nucwords',
  '\ntesting\nucwords\n testing \n ucwords',
  'using\vvertical\vtab',

  //using special chars in sentence
  't@@#$% %test ^test &test *test +test -test',
  '!test ~test `test` =test= @test@test.com',
  '/test/r\test\ucwords\t\y\y\u\3 \yy\ /uu/',
  
  //only special chars
  '!@#$%^&*()_+=-`~'
];

// loop through the $str_array array to test ucwords on each element
$iteration = 1;
for($index = 0; $index < count($str_array); $index++) {
  echo "-- Iteration $iteration --\n";
  var_dump( ucwords($str_array[$index]) );
  $iteration++;
}

echo "Done\n";
}
