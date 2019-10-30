<?hh

// string input[, string delimiter[, string enclosure[, string escape]]]
<<__EntryPoint>> function main(): void {
var_dump(str_getcsv('"f", "o", ""'));
print "-----\n";
var_dump(str_getcsv('foo||bar', '|'));
print "-----\n";
var_dump(str_getcsv('foo|bar', '|'));
print "-----\n";
var_dump(str_getcsv('|foo|-|bar|', '-', '|'));
print "-----\n";
var_dump(str_getcsv('|f.|.|bar|.|-|-.|', '.', '|', '-'));
print "-----\n";
var_dump(str_getcsv('.foo..bar.', '.', '.', '.'));
print "-----\n";
var_dump(str_getcsv('.foo. .bar.', '   ', '.', '.'));
print "-----\n";
var_dump(str_getcsv((string)'1foo1 1bar111', (string)'   ', (string)'1   ', (string)'\  '));
print "-----\n";
var_dump(str_getcsv('.foo  . .  bar  .', ' ', '.', ''));
print "-----\n";
var_dump(str_getcsv('" "" "', ' '));
print "-----\n";
var_dump(str_getcsv(''));
print "-----\n";
}
