<?hh


function main($s) :mixed{
  unset($s[0]);
}

<<__EntryPoint>>
function main_unset_string() :mixed{
main('yo');
}
