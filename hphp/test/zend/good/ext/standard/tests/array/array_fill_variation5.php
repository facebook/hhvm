<?hh
/* Prototype  : array array_fill(int $start_key, int $num, mixed $val)
 * Description: Create an array containing num elements starting with index start_key each initialized to val 
 * Source code: ext/standard/array.c
 */

/*
 * testing array_fill() by passing different types of array  values for 'val' argument
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_fill() : usage variations ***\n";

// Initialise function arguments not being substituted 
$start_key = 0;
$num = 2;


//array of different types of array values for 'val' argument  
$values = varray[
           
  /* 1  */  varray[],
            varray[1 , 2 , 3 , 4],
            darray[1 => "Hi" , 2 => "Hello"],
            varray["Saffron" , "White" , "Green"],
  /* 5  */  darray['color' => 'red' , 'item' => 'pen'],
            darray[ 'color' => 'red' , 2 => 'green ' ],
            darray["colour" => "red" , "item" => "pen"],
            darray[ TRUE => "red" , FALSE => "green" ],
            darray[ true => "red" , FALSE => "green" ],
  /* 10 */  darray[ 1 => "Hi" , "color" => "red" , 'item' => 'pen'],
            darray[ NULL => "Hi", '1' => "Hello" , "1" => "Green"],
            darray[ ""=>1, "color" => "green"], 
  /* 13 */  varray['Saffron' , 'White' , 'Green']
];

// loop through each element of the values array for 'val' argument 
// check the working of array_fill()
echo "--- Testing array_fill() with different types of array values for 'val' argument ---\n";
$counter = 1;
for($i = 0; $i < count($values); $i++)
{
  echo "-- Iteration $counter --\n";
  $val = $values[$i];
  
  var_dump( array_fill($start_key , $num , $val) );

  $counter++;
}

echo "Done";
}
