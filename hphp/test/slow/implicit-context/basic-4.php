<?hh

<<__EntryPoint>>
function main() :mixed{
  include 'implicit.inc';
  try {
    var_dump(IntContext::getContext());
  } catch (InvalidOperationException $e) {
    echo $e->getMessage();
  }
}
