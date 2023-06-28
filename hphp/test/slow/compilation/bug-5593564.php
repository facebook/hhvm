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
$packed = varray[1,2,3];
$mixed = darray[(1 << 33) => varray["value"]];

main($packed);
main($mixed);
main($mixed);
main($mixed);
}
