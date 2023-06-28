<?hh

function f() :mixed{
  throw new Exception("s");
}

function g() :mixed{
  try {
    f();
  } catch (Exception $e) {}
}

function h() :mixed{
  g();
  echo "exiting\n";
  exit(1);
}

function k() :mixed{
  h();
}

<<__EntryPoint>>
function main() :mixed{
  k();
}
