<?hh

function main($s) :mixed{
  var_dump(wordwrap($s, 10));
}



<<__EntryPoint>>
function main_builtin_defaults() :mixed{
main("hello goodbye");
}
