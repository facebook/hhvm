<?php

function handler($name, $obj, $args, $data, &$done) {
  echo "----HANDLER----\n";
  var_dump($name, $obj, $args, $data, $done);
  echo "---------------\n";
}

function passthrough_handler($name, $obj, $args, $data, &$done) {
  echo "----HANDLER----\n";
  var_dump($name, $obj, $args, $data, $done);
  $done = false;
  echo "---------------\n";
}

class MagicCall {
  public function __call($name, $args) {
    echo "magic call! ";
    var_dump($name, $args);
    echo "\n";
  }
}



function frap($arg) {
  echo "frap $arg\n";
}

// Call once normally first; make sure translator can handle it
frap('claptrap');

// Intercept a function
fb_intercept('frap', 'handler', 'data');
frap('claptrap');
call_user_func('frap', 'callfunc');

fb_intercept('frap', 'passthrough_handler');
frap('claptrap');
call_user_func('frap', 'callfunc');

// Replace with closure
fb_intercept('frap', function () { echo "Closure! wooooo\n"; });
frap('claptrap');

// Replace with __call-having object
$mc = new MagicCall();
fb_intercept('frap', array($mc, 'i_dont_exist'));
frap('claptrap');

// Reset
fb_intercept('frap', null);
frap('claptrap');

class Blark {
  public static function sfrap() {
    echo "static frap " . get_called_class() . "\n";
  }
  public function frap() {
    echo "non-static frap\n";
  }
}

class SubBlark extends Blark {}
class SubBlark2 extends Blark {}
function main () {

  $mc = new MagicCall();

  // Intercept static method
  fb_intercept('SubBlark2::sfrap', 'handler');
  Blark::sfrap();
  call_user_func(array('Blark', 'sfrap'));
  SubBlark::sfrap();
  call_user_func(array('SubBlark', 'sfrap'));
  SubBlark2::sfrap();
  call_user_func(array('SubBlark2', 'sfrap'));

  fb_intercept('Blark::sfrap', 'handler');
  Blark::sfrap();
  call_user_func(array('Blark', 'sfrap'));
  SubBlark::sfrap();
  call_user_func(array('SubBlark', 'sfrap'));
  SubBlark2::sfrap();
  call_user_func(array('SubBlark2', 'sfrap'));

  fb_intercept('Blark::sfrap', 'passthrough_handler');
  Blark::sfrap();
  call_user_func(array('Blark', 'sfrap'));

  fb_intercept('Blark::sfrap', array($mc, 'i_dont_exist_either'));
  Blark::sfrap();
  call_user_func(array('Blark', 'sfrap'));

  // Intercept non-static method
  $b = new Blark();
  fb_intercept('Blark::frap', 'handler');
  $b->frap();
  call_user_func(array($b, 'frap'));

  fb_intercept('Blark::frap', 'passthrough_handler');
  $b->frap();
  call_user_func(array($b, 'frap'));

  fb_intercept('Blark::frap', array($mc, 'i_dont_exist_either'));
  $b->frap();
  call_user_func(array($b, 'frap'));

  // MULTI-INTERCEPT!
  fb_intercept('frap', 'handler');
  fb_intercept('handler', 'passthrough_handler');
  frap('claptrap');

  // Reset all
  fb_intercept('', null);
  frap('claptrap');
  Blark::sfrap();
  $b->frap();

  // Intercept __call
  fb_intercept('MagicCall::__call', 'handler');
  $mc->blark('hi');
  fb_intercept('MagicCall::__call', 'passthrough_handler');
  $mc->blark('ho');
}
main();
