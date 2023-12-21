<?hh
/* Prototype  : proto array array_fill(int start_key, int num, mixed val)
 * Description: Create an array containing num elements starting with index start_key each initialized to val 
 * Source code: ext/standard/array.c
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_fill() : basic functionality ***\n";

// calling the array_fill with all possible valid values for 'val' argument
$start_key = 0 ;
$num = 2;
$heredoc = <<<HERE_DOC
Hello
HERE_DOC;

// array of possible valid values for 'val' argument
$values = vec[
  
  /* 1  */  NULL,
            0,
            1,
  /* 4  */  1.5,
            'hi',
            "hi",
  /* 7  */  $heredoc

];

$counter = 1;
for($i = 0; $i < count($values); $i ++)
{
  echo "-- Iteration $counter --\n";
  $val = $values[$i];

  var_dump( array_fill($start_key,$num,$val) );

  $counter++;
}  

echo "Done";
}
