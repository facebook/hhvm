<?hh

trait tttt {}
class cccc {}
interface iiii {}

<<__EntryPoint>>
function main() :mixed{
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

  foreach (vec['tttt', 'cccc', 'iiii', 'zzzz'] as $n) {
    var_dump(class_exists($n));
    var_dump(interface_exists($n));
    var_dump(trait_exists($n));
  }
}
