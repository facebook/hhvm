<?hh
/* $Id$ */
<<__EntryPoint>> function main(): void {
$xmlstring = '<?xml version="1.0" encoding="UTF-8"?>
<books></books>';

$reader = new XMLReader();
$reader->XML($xmlstring);

// Only go through
while ($reader->read()) {
    echo $reader->name."\n";
}
echo "===DONE===\n";
}
