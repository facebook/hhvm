<?hh

function adjustDoctype($xml) :mixed{
    return str_replace(vec[">
<","DOCTYPE HTML",'<p>','</p>'],vec["><","DOCTYPE html",'',''],$xml);
}


<<__EntryPoint>>
function main_1674() :mixed{
$dom = new DOMDocument;
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
