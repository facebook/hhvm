<?hh

class Ref { public function __construct(public $val) {} }

function replace_variables($text, $params) {
  $text = new Ref($text);
  $params = new Ref($params);
  $count = -1;

  $c = function($matches) use ($params, $text) {
    $__val = $params->val;
    $text->val = preg_replace( '/(\?)/', array_shift(&$__val), $text->val, 1);
    $params->val = $__val;
  };

  preg_replace_callback( '/(\?)/', $c, $text->val , -1, inout $count);

  return $text->val;
}
<<__EntryPoint>> function main(): void {
echo replace_variables('a=?', array('0')) . "\n";
echo replace_variables('a=?, b=?', array('0', '1')) . "\n";
echo replace_variables('a=?, b=?, c=?', array('0', '1', '2')) . "\n";
echo "Done\n";
}
