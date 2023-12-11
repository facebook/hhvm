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

$time_formats = dict[

      'Lowercase Ante meridiem and post meridiem' => 'a',
      'Uppercase Ante meridiem and post meridiem' => 'a',
      'Swatch Internet time' => 'B',
      '12-hour format without leading zeros' => 'g',
      '24-hour format without leading zeros' => 'G',
      '12-hour format with leading zeros' => 'h',
      '24-hour format with leading zeros' => 'H',
      'Minutes with leading zeros' => 'i',
      'Seconds with leading zeros' => 's',
      'Milliseconds' => 'u',
];

foreach($time_formats as $key =>$value) {
      echo "\n--$key--\n";
      var_dump( gmdate($value) );
      var_dump( gmdate($value, $timestamp) );
}

echo "===DONE===\n";
}
