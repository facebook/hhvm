<?hh

function f(mixed $x) {
  $x as int;
  // HHBBC should remove the is_int check and bring echo done to this level
  if (is_int($x)) {
    echo "done\n";
  } else {
    echo "unreachable\n";
  }
}

f(1);
try { f(true); } catch (Exception $_) {}
try { f("wow"); } catch (Exception $_) {}
