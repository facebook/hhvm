<?hh

function main() :mixed{
  var_dump(dict(\array_slice(dict(range(1, 10)), 3, 2, true)));
  var_dump(dict(array_combine(range(1, 10), range(11, 20))));
}

<<__EntryPoint>>
function main_crash_10884951() :mixed{
main();
}
