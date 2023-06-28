<?hh

function f() :mixed{ throw new Exception("Exn coming from f"); }
function y() :mixed{ throw new Exception("Exn coming from y"); }

function g() :mixed{
  try {
    f();
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
}

function z() :mixed{
  try {
    y();
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
}

function handler($event, $name, $info) :mixed{
  if ($event === 'exit' && $name === 'f') {
    z();
  }
}

<<__EntryPoint>>
function main() :mixed{
  fb_setprofile('handler');
  g();
}
