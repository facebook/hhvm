<?hh

function __autoload($n) {
  var_dump($n);
  include 'class_interface_trait_exists1.inc';
}

trait tttt {}
class cccc {}
interface iiii {}
if (0) {
  include 'class_interface_trait_exists2.inc';
}
function main() {
  var_dump(class_exists('fooo', false));
  var_dump(class_exists('fooo'));
  var_dump(class_exists('tttt'));
  var_dump(class_exists('cccc'));
  var_dump(class_exists('iiii'));
  var_dump(class_exists('zzzz'));

  var_dump(interface_exists('tttt'));
  var_dump(interface_exists('cccc'));
  var_dump(interface_exists('iiii'));
  var_dump(interface_exists('zzzz'));

  var_dump(trait_exists('tttt'));
  var_dump(trait_exists('cccc'));
  var_dump(trait_exists('iiii'));
  var_dump(trait_exists('zzzz'));

  foreach (array('tttt', 'cccc', 'iiii', 'zzzz') as $n) {
    var_dump(class_exists($n));
    var_dump(interface_exists($n));
    var_dump(trait_exists($n));
  }
}
main();
