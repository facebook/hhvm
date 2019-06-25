<?hh <<__EntryPoint>> function main(): void {
$text = "Some text";
$string = "$text $text $text $text";
echo wordwrap($string, 9);
}
