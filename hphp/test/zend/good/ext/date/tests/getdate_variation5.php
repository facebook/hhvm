<?hh
/* Prototype  : array getdate([int timestamp])
 * Description: Get date/time information
 * Source code: ext/date/php_date.c
 * Alias to functions:
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing getdate() : usage variation ***\n";

//Timezones with required data for date_sunrise
$inputs = vec[
        //GMT-11
        "Pacific/Samoa",
        //GMT-9
        "US/Alaska",
        //GMT-0
        "Africa/Casablanca",
        //GMT+4
        "Europe/Moscow",
        //GMT+8
        "Asia/Hong_Kong",
        //GMT+10
        "Australia/Brisbane",
        //GMT+12
        "Pacific/Wallis",
];

// loop through each element of the array for timestamp
foreach($inputs as $timezone) {
      echo "\n--$timezone--\n";
      date_default_timezone_set($timezone);
      var_dump( getdate(0) );
};
echo "===DONE===\n";
}
