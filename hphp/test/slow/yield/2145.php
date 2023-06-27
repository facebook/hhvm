<?hh

function fruit(): Iterator<string> {
 echo "sadpanda, no fruit";
 yield break;
 }


 <<__EntryPoint>>
function main_2145() {
foreach (fruit() as $fruit) {
 var_dump($fruit);
}
}
