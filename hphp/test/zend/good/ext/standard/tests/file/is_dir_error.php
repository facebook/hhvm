<?php
/* Prototype: bool is_dir ( string $filename );
 *  Description: Tells whether the filename is a regular file
 *               Returns TRUE if the filename exists and is a regular file
 */

echo "*** Testing is_dir() error conditions ***";
var_dump( is_dir() );  // Zero No. of args

$file_path = getenv('HPHP_TEST_TMPDIR') ?? dirname(__FILE__);
$dir_name = $file_path."/is_dir_error";
mkdir($dir_name);
var_dump( is_dir($dir_name, "is_dir_error1") ); // args > expected no.of args

/* Non-existing dir */
var_dump( is_dir("/no/such/dir") );

echo "*** Done ***";
?>

<?php error_reporting(0); ?>
<?php
$file_path = getenv('HPHP_TEST_TMPDIR') ?? dirname(__FILE__);
rmdir($file_path."/is_dir_error");
?>
