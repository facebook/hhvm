<?hh

namespace AutoimportTest;

include 'autoimport_defs.inc';

class ContainerExt extends Container {}
<<__EntryPoint>> function main(): void {
\var_dump(Container::class);

$a = new ContainerExt;
\var_dump(\get_class($a));
\var_dump(\get_parent_class($a));
}
