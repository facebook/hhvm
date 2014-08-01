<?php
$filename = __DIR__ . '\\a.php';
$content = '<?php echo "Dummy Content.\n" ?>';
file_put_contents($filename, $content);
$linkname = __DIR__ . '\\a_link.php';
link("$filename", "$linkname");
var_dump(file_exists("$linkname"));
$linkcontent = file_get_contents($linkname);
var_dump($content == $linkcontent);
unlink($filename);
unlink($linkname);
?>