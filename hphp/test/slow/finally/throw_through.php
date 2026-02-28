<?hh
<<__EntryPoint>>
function foo() :mixed{
  try {
    throw new Exception('foo');
  } finally {
    var_dump("me first!");
  }
}
