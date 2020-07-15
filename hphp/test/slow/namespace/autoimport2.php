<?hh

namespace AutoimportTest;



class TraversableExt extends Traversable {}
<<__EntryPoint>> function main(): void {
include 'autoimport_defs.inc';
\var_dump(Traversable::class);

$a = new TraversableExt;
\var_dump(\get_class($a));
\var_dump(\get_parent_class($a));
}
