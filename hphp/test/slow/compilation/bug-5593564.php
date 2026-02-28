<?hh

function main($arr) :mixed{
  try {
    if ($arr[1 << 33]) {
      var_dump($arr[1 << 33]);
    } else {
      echo "no\n";
    }
  } catch (Exception $e) { echo $e->getMessage()."\n"; }
}


<<__EntryPoint>>
function main_bug_5593564() :mixed{
$packed = vec[1,2,3];
$mixed = dict[(1 << 33) => vec["value"]];

main($packed);
main($mixed);
main($mixed);
main($mixed);
}
