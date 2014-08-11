<?php
$d = new DOMDocument();
$str = <<<EOD
<html>
<head>
</head>
<body>
<p>Hi.<br/>there</p>
</body>
</html>
EOD;
$d->loadHTML($str);
$e = $d->getElementsByTagName("p");
$e = $e->item(0);
echo $d->saveXml($e),"\n";
echo $d->saveHtml($e),"\n";
