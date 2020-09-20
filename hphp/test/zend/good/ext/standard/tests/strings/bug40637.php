<?hh
<<__EntryPoint>> function main(): void {
$html = '<span title="Bug \' Trigger">Text</span>';
var_dump(strip_tags($html));

echo "Done\n";
}
