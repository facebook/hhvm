<?hh
<<__EntryPoint>> function main(): void {
$html = "<div>Bug ' Trigger</div> Missing Text";
var_dump(strip_tags($html));

echo "Done\n";
}
