<?hh
class Thing {}
function boom()
{
    $reader = xml_parser_create();
    $thing = new Thing();
    xml_set_object($reader, $thing);
    die("ok\n");
    xml_parser_free($reader);
}
<<__EntryPoint>>
function main_entry(): void {
  boom();
}
