<?hh
/* Prototype  : string strftime(string format [, int timestamp])
 * Description: Format a local time/date according to locale settings
 * Source code: ext/date/php_date.c
 * Alias to functions:
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing strftime() : usage variation ***\n";

date_default_timezone_set("Asia/Calcutta");
// Initialise function arguments not being substituted (if any)
$timestamp = mktime(8, 8, 8, 8, 8, 2008);

//array of values to iterate over
$inputs = dict[
      'Abbreviated month name' => "%b",
      'Full month name' => "%B",
      'Month as decimal' => "%m",
];

// loop through each element of the array for timestamp

foreach($inputs as $key =>$value) {
      echo "\n--$key--\n";
      var_dump( strftime($value) );
      var_dump( strftime($value, $timestamp) );
};

echo "===DONE===\n";
}
