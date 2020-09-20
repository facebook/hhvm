<?hh

<<__EntryPoint>> function main(): void {
$a = 'a';
$b = 'A';
setlocale (LC_COLLATE, 'C');
$result = strcoll($a, $b);
if($result > 0) {
    echo "Pass\n";
}
}
