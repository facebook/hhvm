<?hh
function testme() :mixed{
  $data='a=1&b=2&c=3';
  echo 'parse_data(): ',$data,EOL;
  $output = null;
  parse_str($data, inout $output);
  if (isset($output['b'])) {
    echo 'isset b='.$output['b'],EOL;
  } else {
    echo 'notset b='.$output['b'],EOL;
  }
  if (isset($output['b'])) {
    echo 'isset b='.$output['b'],EOL;
  } else {
    echo 'no b='.$output['b'],EOL;
  }
}
const EOL = "\n";

<<__EntryPoint>>
function main_parse_str() :mixed{

$data='a=1&b=2&c=3';
echo 'global scope',EOL;
echo 'parse_data(): ',$data,EOL;
$output = null;
parse_str($data, inout $output);
if (isset($output['b'])) {
  echo 'isset b='.$output['b'],EOL;
} else {
  echo 'no b='.$output['b'],EOL;
}
echo EOL;

echo 'within function',EOL;
testme();
}
