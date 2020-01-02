<?hh

function f() { throw new Exception("Exn coming from f"); }
function y() { throw new Exception("Exn coming from y"); }

function g() {
  try {
    f();
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
}

function z() {
  try {
    y();
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
}

function handler($event, $name, $info) {
  if ($event === 'exit' && $name === 'f') {
    z();
  }
}

<<__EntryPoint>>
function main() {
  fb_setprofile('handler');
  g();
}
