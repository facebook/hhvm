<?hh
/* Prototype  : string strftime(string format [, int timestamp])
 * Description: Format a local time/date according to locale settings
 * Source code: ext/date/php_date.c
 * Alias to functions:
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing strftime() : usage variation ***\n";

// Initialise function arguments not being substituted (if any)
setlocale(LC_ALL, "en_US");
date_default_timezone_set("Asia/Calcutta");
$timestamp = mktime(18, 8, 8, 8, 8, 2008);


//array of values to iterate over
$inputs = darray[
      'Hour as decimal by 24-hour format' => "%H",
      'Hour as decimal by 12-hour format' => "%I",
      'Minute as decimal number' => "%M",
      'AM/PM format for a time' => "%p",
      'Second as decimal number' => "%S",
];

// loop through each element of the array for timestamp

foreach($inputs as $key =>$value) {
      echo "\n--$key--\n";
      var_dump( strftime($value) );
      var_dump( strftime($value, $timestamp) );
};

echo "===DONE===\n";
}
