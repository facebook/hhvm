<?hh

function f() {
  throw new Exception("s");
}

function g() {
  try {
    f();
  } catch (Exception $e) {}
}

function h() {
  g();
  echo "exiting\n";
  exit(1);
}

function k() {
  h();
}

<<__EntryPoint>>
function main() {
  k();
}
