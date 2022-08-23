<?hh

class Foo {}
type Bar = int;

<<__EntryPoint>>
function main() {
  var_dump(json_decode(HH\embed_type_decl<Foo>()));
  var_dump(json_decode(HH\embed_type_decl<Bar>()));
  var_dump(json_decode(HH\embed_type_decl<DoesNotExist>()));
}
