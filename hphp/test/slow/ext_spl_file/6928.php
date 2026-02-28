<?hh


<<__EntryPoint>>
function main_6928() :mixed{
$text =  "a\tb\tc\t\n";
$text .= "a\tb\tc\n";
$text .= "1\t2\t";

$file = new SplFileObject('data://text/plain,' . $text);
$file->setFlags(SplFileObject::DROP_NEW_LINE);

foreach($file as $row) {
    echo "+".$row."+\n";
}

$file = null;
}
