<?hh
/* Prototype  : int iconv_strlen(string str [, string charset])
 * Description: Get character numbers of a string
 * Source code: ext/iconv/iconv.c
 */

/*
 * Test iconv_strlen by passing different data types as $str argument
 */

// get a class
class classA
{
  public function __toString() :mixed{
    return "Class A object";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing iconv_strlen() : usage variations ***\n";

// Initialise function arguments not being substituted
$encoding = 'utf-8';


// heredoc string
$heredoc = <<<EOT
hello world
EOT;

// get a resource variable
$fp = fopen(__FILE__, "r");

// unexpected values to be passed to $str argument
$inputs = dict[

       // int data
/*1*/
     'int 0' => 0,
     'int 1' => 1,
     'int 12345' => 12345,
     'int -12345' => -12345,

       // float data
/*5*/
     'float 10.5' => 10.5,
     'float -10.5' => -10.5,
     'float 12.3456789000e10' => 12.3456789000e10,
     'float 12.3456789000e-10' => 12.3456789000e-10,
     'float .5' => .5,

       // null data
/*10*/
      'uppercase NULL' => NULL,
      'lowercase null' => null,

       // boolean data
/*12*/
      'lowercase true' => true,
      'lowercase false' =>false,
      'uppercase TRUE' =>TRUE,
      'uppercase FALSE' =>FALSE,

       // empty data
/*16*/
      'empty string DQ' => "",
      'empty string SQ' => '',

       // string data
/*18*/
      'string DQ' => "string",
      'string SQ' => 'string',
      'mixed case string' => "sTrInG",
      'heredoc' => $heredoc,

       // object data
/*21*/
      'instance of class' => new classA(),

       // resource variable
/*22*/
       'resource' => $fp
];

// loop through each element of $inputs to check the behavior of iconv_strlen()
$iterator = 1;
foreach($inputs as $key =>$value) {
  echo "\n--$key--\n";
  try { var_dump( iconv_strlen($value, $encoding)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  $iterator++;
};

fclose($fp);
echo "===DONE===\n";
}
