<?hh

/* Prototype  : string escapeshellarg  ( string $arg  )
 * Description:  Escape a string to be used as a shell argument.
 * Source code: ext/standard/exec.c
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing escapeshellarg() : usage variations ***\n";


$inputs = vec[
       // empty data
/*17*/ "",
       '',
];

// loop through each element of $inputs to check the behaviour of escapeshellarg()
$iterator = 1;
foreach($inputs as $input) {
    echo "\n-- Iteration $iterator --\n";
    try { var_dump(escapeshellarg($input)); } catch (Exception $e) { var_dump($e->getMessage()); }
    $iterator++;
};
echo "===Done===";
}
