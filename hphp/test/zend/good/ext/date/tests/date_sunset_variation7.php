<?hh
/* Prototype  : mixed date_sunset(mixed time [, int format [, float latitude [, float longitude [, float zenith [, float gmt_offset]]]]])
 * Description: Returns time of sunrise for a given day and location
 * Source code: ext/date/php_date.c
 * Alias to functions:
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing date_sunset() : usage variation ***\n";

//Timezones with required data for date_sunrise
$inputs = dict[
        //Timezone with Latitude, Longitude and GMT offset
        "Pacific/Samoa" => dict["Latitude" => -14.24, "Longitude" => -170.72, "GMT" => -11.0],
        "US/Alaska" => dict["Latitude" => 61.0, "Longitude" => -150.0 , "GMT" => -9.0],
        "America/Chicago" => dict["Latitude" => 41.85, "Longitude" => -87.65 , "GMT" => -5.0],
        "America/Montevideo" => dict["Latitude" => -34.88, "Longitude" => -56.18 , "GMT" => -3.0],
        "Africa/Casablanca" => dict["Latitude" => 33.65, "Longitude" => -7.58, "GMT" => 0.0],
        "Europe/Moscow" => dict["Latitude" => 55.75, "Longitude" => 37.58, "GMT" => 4.0],
        "Asia/Hong_Kong" => dict["Latitude" => 22.28, "Longitude" => 114.15 , "GMT" => 8.0],
        "Australia/Brisbane" => dict["Latitude" => -27.46, "Longitude" => 153.2 , "GMT" => 10.0],
        "Pacific/Wallis" => dict["Latitude" => -13.3, "Longitude" => -176.16, "GMT" => 12.0],
];

foreach($inputs as $timezone => $value) {
     echo "\n--$timezone--\n";
     date_default_timezone_set($timezone);
     $time = mktime(8, 8, 8, 8, 11, 2008);
     var_dump( date_sunset($time, SUNFUNCS_RET_STRING, $value["Latitude"], $value["Longitude"], 90.0, $value["GMT"] ));
     $time = mktime(8, 8, 8, 8, 12, 2008);
     var_dump( date_sunset($time, SUNFUNCS_RET_STRING, $value["Latitude"], $value["Longitude"], 90.0, $value["GMT"]) );
}
echo "===DONE===\n";
}
