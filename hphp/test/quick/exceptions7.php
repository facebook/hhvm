<?hh

function handler($name, $obj, inout $args, $data, inout $done) {
  echo "----HANDLER----\n";
  var_dump($name, $obj, $args, $data, $done);
  echo "---------------\n";
  throw new Exception;
}

function passthrough_handler($name, $obj, inout $args, $data, inout $done) {
  echo "----HANDLER----\n";
  var_dump($name, $obj, $args, $data, $done);
  $done = false;
  echo "---------------\n";
  throw new Exception;
}

class MagicCall {
  public function __call($name, $args) {
    echo "magic call! ";
    var_dump($name, $args);
    echo "\n";
    throw new Exception;
  }
}

function frap($arg) {
  echo "frap $arg\n";
}

class Blark {
  public static function sfrap() {
    echo "static frap\n";
  }
  public function frap() {
    echo "non-static frap\n";
  }
}
<<__EntryPoint>>
function main_entry(): void {

  // Intercept a function
  fb_intercept('frap', 'handler', 'data');
  try {
    call_user_func(fun('frap'), 'callfunc');
  } catch (Exception $e) {
    echo "caught call user func 1\n";
  }

  fb_intercept('frap', 'passthrough_handler');
  try {
    call_user_func(fun('frap'), 'callfunc');
  } catch (Exception $e) {
    echo "caught call user func 2\n";
  }

  // Replace with closure
  fb_intercept('frap', function ($_1, $_2, inout $_3, $_4, inout $_5) {
                         echo "Closure! wooooo\n";
                         throw new Exception;
                       });
  try {
    call_user_func(fun("frap"), 'claptrap');
  } catch (Exception $e) {
    echo "caught closure 1\n";
  }

  $mc = new MagicCall();

  // Intercept static method
  fb_intercept('Blark::sfrap', 'handler');
  try {
    call_user_func(varray['Blark', 'sfrap']);
  } catch (Exception $e) {
    echo "caught static call 1\n";
  }

  fb_intercept('Blark::sfrap', 'passthrough_handler');
  try {
    call_user_func(varray['Blark', 'sfrap']);
  } catch (Exception $e) {
    echo "caught static call 2\n";
  }

  // Intercept non-static method
  $b = new Blark();
  fb_intercept('Blark::frap', 'handler');
  try {
    call_user_func(varray[$b, 'frap']);
  } catch (Exception $e) {
    echo "caught non-static call 1\n";
  }

  fb_intercept('Blark::frap', 'passthrough_handler');
  try {
    call_user_func(varray[$b, 'frap']);
  } catch (Exception $e) {
    echo "caught non-static call 2\n";
  }

  // MULTI-INTERCEPT!
  fb_intercept('frap', 'handler');
  fb_intercept('handler', 'passthrough_handler');
  try {
    call_user_func(fun("frap"), 'claptrap');
  } catch (Exception $e) {
    echo "caught double intercept 1\n";
  }

  // Intercept __call
  fb_intercept('MagicCall::__call', 'handler');
  try {
    call_user_func(varray[$mc, "blark"], 'hi');
  } catch (Exception $e) {
    echo "caught __call 1\n";
  }
  fb_intercept('MagicCall::__call', 'passthrough_handler');
  try {
    call_user_func(varray[$mc, "blark"], 'ho');
  } catch (Exception $e) {
    echo "caught __call 2\n";
  }
}
