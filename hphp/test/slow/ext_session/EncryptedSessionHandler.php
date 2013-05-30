<?php

class EncryptedSessionHandler extends SessionHandler {
  private $key;

  public function __construct($key) {
    $this->key = $key;
  }

  public function read($id) {
    $data = parent::read($id);
    var_dump($data);
    return mcrypt_decrypt(MCRYPT_3DES, $this->key, $data, MCRYPT_MODE_ECB);
  }

  public function write($id, $data) {
    $data = mcrypt_encrypt(MCRYPT_3DES, $this->key, $data, MCRYPT_MODE_ECB);
    var_dump($data);
    return parent::write($id, $data);
  }
}

ini_set('session.save_handler', 'files');
$handler = new EncryptedSessionHandler('mykey');
session_set_save_handler($handler, true);
session_start();

$_SESSION['a'] = 'A';
var_dump($_SESSION['a']);
