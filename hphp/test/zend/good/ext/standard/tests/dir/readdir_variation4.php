<?hh
/* Prototype  : string readdir([resource $dir_handle])
 * Description: Read directory entry from dir_handle
 * Source code: ext/standard/dir.c
 */

/*
 * Pass a directory handle pointing to a directory that contains
 * files with different file names to test how readdir() reads them
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing readdir() : usage variations ***\n";

$dir_path = sys_get_temp_dir().'/'.'readdir_variation4' . '/';
mkdir($dir_path);

// heredoc string
$heredoc = <<<EOT
hd_file
EOT;

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

       // empty data
/*10*/ "",
       'Array',

       // string data
/*12*/ "double_file",
       'single_file',
       $heredoc,
];

$iterator = 1;
foreach($inputs as $key => $input) {
    echo "\n-- Iteration $iterator --\n";
    $handle = "fp{$iterator}";
    var_dump( $handle = fopen(@"$dir_path$input.tmp", 'w') );
    try { var_dump( fwrite($handle, $key)); } catch (Exception $e) { var_dump($e->getMessage()); }
    fclose($handle);
    $iterator++;
};

echo "\n-- Call to readdir() --\n";
$dir_handle = opendir($dir_path);
$contents = vec[];
while(FALSE !== ($file = readdir($dir_handle))){

    // different OS order files differently so will
    // store file names into an array so can use sorted in expected output
    $contents[] = $file;

    // remove files while going through directory
    @unlink($dir_path . $file);
}

// more important to check that all contents are present than order they are returned in
sort(inout $contents);
var_dump($contents);

closedir($dir_handle);
echo "===DONE===\n";

rmdir($dir_path);
}
