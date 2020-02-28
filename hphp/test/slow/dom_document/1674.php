<?hh

function adjustDoctype($xml) {
    return str_replace(varray[">
<","DOCTYPE HTML",'<p>','</p>'],varray["><","DOCTYPE html",'',''],$xml);
}


<<__EntryPoint>>
function main_1674() {
$dom = new domdocument;
$html = <<<EOM
<html><head>
<title>Hello world</title>
</head>
<body>
This is a not well-formed<br>
html files with undeclared entities&nbsp;
</body>
</html>
EOM;
$dom->loadHTML($html);
print  "--- save as XML
";

print adjustDoctype($dom->saveXML());
print  "--- save as HTML
";

print adjustDoctype($dom->saveHTML());
}
