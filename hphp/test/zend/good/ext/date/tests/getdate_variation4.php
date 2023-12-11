<?hh
/* Prototype  : array getdate([int timestamp])
 * Description: Get date/time information
 * Source code: ext/date/php_date.c
 * Alias to functions:
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing getdate() : usage variation ***\n";

//Set the default time zone
date_default_timezone_set("Asia/Calcutta");

//array of values to iterate over
$inputs = dict[

    //Year wise time stamps
    '01 Jan 1970' => 0,
    '01 Jan 1971' => 31536000,
    '01 Jan 1972' => 63072000,
    '01 Jan 1973' => 94694400,
];

// loop through each element of the array for timestamp

foreach($inputs as $key =>$value) {
      echo "\n--$key--\n";
      var_dump( getdate($value) );
};

echo "===DONE===\n";
}
