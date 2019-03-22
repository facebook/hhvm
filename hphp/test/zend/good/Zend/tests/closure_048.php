<?php

class Ref { public function __construct(public $val) {} }

function replace_variables($text, $params) {
  $text = new Ref($text);
  $params = new Ref($params);

  $c = function($matches) use ($params, $text) {
    $text->val = preg_replace( '/(\?)/', array_shift(&$params->val), $text->val, 1);
  };

  preg_replace_callback( '/(\?)/', $c, $text->val );

  return $text->val;
}

echo replace_variables('a=?', array('0')) . "\n";
echo replace_variables('a=?, b=?', array('0', '1')) . "\n";
echo replace_variables('a=?, b=?, c=?', array('0', '1', '2')) . "\n";
echo "Done\n";
