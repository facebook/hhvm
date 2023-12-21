<?hh
/* Prototype  : mixed json_decode  ( string $json  [, bool $assoc  ] )
 * Description: Decodes a JSON string
 * Source code: ext/json/php_json.c
 * Alias to functions:  */
<<__EntryPoint>> function main(): void {
echo "*** Testing json_decode() : basic functionality ***\n";

// array with different values for $string
$inputs =  vec[
        '0',
        '123',
        '-123',
        '2147483647',
        '-2147483648',
        '123.456',
        '1230',
        '-1230',
        'true',
        'false',
        'null',
        '"abc"',
        '"Hello World\r\n"',
        '[]',
        '[1,2,3,4,5]',
        '{"myInt":99,"myFloat":123.45,"myNull":null,"myBool":true,"myString":"Hello World"}',
        '{"Jan":31,"Feb":29,"Mar":31,"April":30,"May":31,"June":30}',
        '""',
        '{}'
];

// loop through with each element of the $inputs array to test json_decode() function
$count = 1;
foreach($inputs as $input) {
  echo "-- Iteration $count --\n";
  var_dump(json_decode($input));
   var_dump(json_decode($input, TRUE));
  $count ++;
}
echo "===Done===";
}
