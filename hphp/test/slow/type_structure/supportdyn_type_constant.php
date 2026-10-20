<?hh

class SupportdynRepro {
  const type F = (function(int): void);
  const type Wrapped = supportdyn<this::F>;
}

<<__EntryPoint>>
function main(): void {
  $wrapped = type_structure(SupportdynRepro::class, 'Wrapped');
  var_dump($wrapped['kind']);
  var_dump($wrapped === type_structure(SupportdynRepro::class, 'F'));
}
