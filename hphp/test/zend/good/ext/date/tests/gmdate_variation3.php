<?hh
/* Prototype  : string gmdate(string format [, long timestamp])
 * Description: Format a GMT date/time
 * Source code: ext/date/php_date.c
 * Alias to functions:
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing gmdate() : usage variation ***\n";

// Initialise all required variables
date_default_timezone_set('UTC');
$timestamp = mktime(8, 8, 8, 8, 8, 2008);

//array of values to iterate over
$inputs = dict[

     'Day with leading zeros' => 'd',
     'Day without leading zeros' => 'j',
     'ISO representation' => 'N',
     'Numeric representation of day' => 'w',
     'Day of the year' => 'z'
];

// loop through each element of the array for timestamp

foreach($inputs as $key =>$value) {
      echo "\n--$key--\n";
      var_dump( gmdate($value) );
      var_dump( gmdate($value, $timestamp) );
};

echo "===DONE===\n";
}
