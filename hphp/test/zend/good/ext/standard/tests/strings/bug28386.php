<?php <<__EntryPoint>> function main() {
$text = "Some text";
$string = "$text $text $text $text";
echo wordwrap($string, 9);
}
