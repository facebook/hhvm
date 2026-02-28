<?hh

<<__JitSerdesPriority(1)>>
interface IFooInterface {}

<<__JitSerdesPriority(0)>>
enum class ParentEC : IFooInterface {}

<<__EntryPoint>>
function main() :mixed{
  $is_serialize = ini_get('hhvm.jit_serdes_mode') === 'Serialize';

  if (!$is_serialize) {
    include "ec-serdes.php.inc";
    var_dump(enum_exists('ChildEC'));
  }
}
