<?php
/* Prototype  : mixed opendir(string $path[, resource $context])
 * Description: Open a directory and return a dir_handle 
 * Source code: ext/standard/dir.c
 */

/*
 * Pass different data types as $context argument to opendir() to test behaviour
 */

echo "*** Testing opendir() : usage variation ***\n";


// Initialise function arguments not being substituted (if any)
// create temporary directory for test, removed in CLEAN section
$path = dirname(__FILE__) . "/opendir_variation2";
mkdir($path);


//get an unset variable
$unset_var = 10;
unset ($unset_var);

// get a class
class classA
{
	public function __toString() 
	{
		return "Class A object";
	}
}

// heredoc string
$heredoc = <<<EOT
hello world
EOT;

// get a resource variable
$fp = fopen(__FILE__, "r");

// unexpected values to be passed to $context argument
$inputs = array(

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
       array(),

       // string data
/*19*/ "string",
       'string',
       $heredoc,
       
       // object data
/*22*/ new classA(),

       // undefined data
/*23*/ @$undefined_var,

       // unset data
/*24*/ @$unset_var,

       // resource variable
/*25*/ $fp
);

// loop through each element of $inputs to check the behavior of opendir()
$iterator = 1;
foreach($inputs as $input) {
	echo "\n-- Iteration $iterator --\n";
	var_dump($dh = opendir($path, $input) );#
	if ($dh) {
		closedir($dh);
	}
	$iterator++;
};

fclose($fp);
?>
===DONE===
<?php
$path = dirname(__FILE__) . "/opendir_variation2";
rmdir($path);
?>