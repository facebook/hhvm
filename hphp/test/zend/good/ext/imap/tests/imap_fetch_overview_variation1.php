<?hh
/* Prototype  : array imap_fetch_overview(resource $stream_id, int $msg_no [, int $options])
 * Description: Read an overview of the information in the headers
 * of the given message sequence
 * Source code: ext/imap/php_imap.c
 */

/*
 * Pass different data types as $stream_id argument to imap_fetch_overview() to test behaviour
 */

// get a class
class classA
{
  public function __toString() :mixed{
    return "Class A object";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing imap_fetch_overview() : usage variations ***\n";

// Initialise function arguments not being substituted
$msg_no = 1;


// heredoc string
$heredoc = <<<EOT
hello world
EOT;

// unexpected values to be passed to $stream_id argument
$inputs = vec[

       // int data
/*1*/  0,
       1,
       12345,
       -2345,

       // float data
/*5*/  10.5,
       -10.5,
       12.3456789000e10,
       12.3456789000E-10,
       .5,

       // null data
/*10*/ NULL,
       null,

       // boolean data
/*12*/ true,
       false,
       TRUE,
       FALSE,

       // empty data
/*16*/ "",
       '',
       vec[],

       // string data
/*19*/ "string",
       'string',
       $heredoc,

       // object data
/*22*/ new classA(),


];

// loop through each element of $inputs to check the behavior of imap_fetch_overview()
$iterator = 1;
foreach($inputs as $input) {
  echo "\n-- Testing with first argument value: ";
  var_dump($input);
  try { var_dump( imap_fetch_overview($input, $msg_no) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  $iterator++;
};
echo "===DONE===\n";
}
