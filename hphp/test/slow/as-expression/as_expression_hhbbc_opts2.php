<?hh

function f(mixed $x) :mixed{
  $x as int;
  // HHBBC should remove the is_int check and bring echo done to this level
  if (is_int($x)) {
    echo "done\n";
  } else {
    echo "unreachable\n";
  }
}


<<__EntryPoint>>
function main_as_expression_hhbbc_opts2() :mixed{
f(1);
try { f(true); } catch (Exception $_) {}
try { f("wow"); } catch (Exception $_) {}
}
