<?hh


const AAA = true;
const BBB = false;
const CCC = null;
<<__EntryPoint>>
function main_1595() :mixed{
if (AAA){
  echo "AAA";
}
 else {
  echo "!AAA";
}
if (BBB) {
  echo "BBB";
}
 else {
  echo "!BBB";
}
if (CCC) {
  echo "CCC";
}
 else {
  echo "!CCC";
}
$a = AAA ? "AAA" : "!AAA";
$b = BBB ? "BBB" : "!BBB";
$c = CCC ? "CCC" : "!CCC";
echo "$a$b$c\n";
}
