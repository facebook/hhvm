<?php

class B {

  public function __construct() {
  }

  public function __destruct() {
    $this->deregister();
  }

  private function deregister() {
    fb_intercept('', false);
  }

  public function run() {
    echo "In B\n";
  }
}

class A {

  public function __construct() {
    $this->register();
  }

  public function __destruct() {
    $this->deregister();
  }

  private function register() {
    $x = new B();
    $proxy = function($name, $obj, $params, $data) {
      echo "In proxy\n";
      $data->run();
    };

    fb_intercept('mail', $proxy, $x);
  }

  private function deregister() {
    fb_intercept('', false);
  }

  public function run() {
    echo "Running\n";
    mail('nothing');
  }
}

function run() {
  $a = new A();
  $a->run();
}

run();
