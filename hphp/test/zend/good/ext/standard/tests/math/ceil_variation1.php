<?hh
/* Prototype  : float ceil  ( float $value  )
 * Description: Round fractions up.
 * Source code: ext/standard/math.c
 */

<<__EntryPoint>> function main(): void {
echo "*** Testing ceil() : usage variations ***\n";

// heredoc string
$heredoc = <<<EOT
abc
xyz
EOT;

// get a resource variable
$fp = fopen(__FILE__, "r");

// unexpected values to be passed to $value argument
$inputs = vec[
       // null data
/* 1*/ NULL,
       null,

       // boolean data
/* 3*/ true,
       false,
       TRUE,
       FALSE,

       // empty data
/* 7*/ "",
       '',
       vec[],

       // string data
/*10*/ "abcxyz",
       'abcxyz}',
       $heredoc,

       // resource variable
/*14*/ $fp
];

// loop through each element of $inputs to check the behaviour of ceil()
$iterator = 1;
foreach($inputs as $input) {
    echo "\n-- Iteration $iterator --\n";
    var_dump(ceil($input));
    $iterator++;
};
fclose($fp);
echo "===Done===";
}
