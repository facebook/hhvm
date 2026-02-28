<?hh
/* Prototype  : string timezone_name_from_abbr  ( string $abbr  [, int $gmtOffset= -1  [, int $isdst= -1  ]] )
 * Description: Returns the timezone name from abbrevation
 * Source code: ext/date/php_date.c
 * Alias to functions:
 */

// define some classes
class classWithToString
{
    public function __toString() :mixed{
        return "Class A object";
    }
}

class classWithoutToString
{
}
<<__EntryPoint>> function main(): void {
echo "*** Testing timezone_name_from_abbr() : usage variation -  unexpected values to second argument \$gmtOffset***\n";

//Set the default time zone
date_default_timezone_set("Europe/London");


// heredoc string
$heredoc = <<<EOT
hello world
EOT;

// add arrays
$index_array = vec[1, 2, 3];
$assoc_array = dict['one' => 1, 'two' => 2];

//array of values to iterate over
$inputs = dict[

      // int data
      'int 0' => 0,
      'int 12345' => 12345,
      'int -12345' => -12345,
];

$abbr= "GMT";
$isdst = 1;

foreach($inputs as $variation =>$gmtOffset) {
      echo "\n-- $variation --\n";
      try { var_dump( timezone_name_from_abbr($abbr, $gmtOffset, $isdst) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
};

echo "===DONE===\n";
}
