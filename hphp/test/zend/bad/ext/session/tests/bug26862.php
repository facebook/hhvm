<?php
session_start();
output_add_rewrite_var('var', 'value');

echo '<a href="file.php">link</a>';

ob_flush();

output_reset_rewrite_vars();
echo '<a href="file.php">link</a>';
?>