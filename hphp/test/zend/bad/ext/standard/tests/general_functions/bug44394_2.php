<?php
ini_set('session.use_only_cookies', 0);

ini_set('session.name', PHPSESSID);


ini_set('session.use_trans_sid', 1);
session_save_path(__DIR__);
session_start();

ob_start();

$string = "<a href='a?q=1'>asd</a>";

output_add_rewrite_var('a', 'b');

echo $string;

ob_flush();

ob_end_clean();

?>
<?php
foreach (glob(__DIR__ . '/sess_*') as $filename) {
  unlink($filename);
}
?>