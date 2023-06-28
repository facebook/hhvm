<?hh


<<__EntryPoint>>
function main_iptcembed_stream() :mixed{
$jpg = base64_encode(file_get_contents(__DIR__."/iptc-data.jpg"));

$s = iptcembed(
  "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
  "data://text/plain;base64,".$jpg
);
var_dump($s);
}
