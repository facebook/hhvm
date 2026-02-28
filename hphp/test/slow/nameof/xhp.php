<?hh

class :xhp:foo {}

<<__EntryPoint>>
function main(): void {
   var_dump(:xhp:foo::class);
   var_dump(nameof :xhp:foo);
}
