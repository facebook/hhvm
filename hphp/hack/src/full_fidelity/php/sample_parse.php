<?hh
// Copyright 2016-present Facebook. All Rights Reserved.
require_once 'full_fidelity_parser.php';
$file = 'sample_parse_input.php';
$root = parse_file_to_editable($file);
print "---file---\n";
print $root->declarations()[0]->full_text();
print "\n";
$text = "<?hh\nfunction foo() {}\nfunction bar() {}";
$root = parse_text_to_editable($text);
print "---text---\n";
print $root->declarations()[1]->full_text();
print "\n";
