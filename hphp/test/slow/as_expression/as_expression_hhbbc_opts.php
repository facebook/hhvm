<?hh

function f() {
  $x = 1;
  // HHBBC should erase the as expression
  $x as int;
  echo "done\n";
}

function g() {
  $x = 1;
  $x as string;
  // as will thow so, HHBBC should erase the echo
  echo "unreachable\n";
}

function h() {
  $x = 1;
  try {
    $x as string;
    // as will thow so, HHBBC should erase the echo
    echo "unreachable\n";
  } catch (Exception $_) {}
  echo "done\n";
}


f();
try {
  g();
} catch (Exception $_) {}
h();
