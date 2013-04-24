<?php
$html = <<< HERE
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
<head>
<title>test</title>
</head>
<body>
<b>PHP!</b>
</body>
</html>

HERE;

echo trim(strip_tags($html, '<b>'))."\n";
?>