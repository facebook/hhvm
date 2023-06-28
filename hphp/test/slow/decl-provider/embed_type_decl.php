<?hh

class Foo {}
type Bar = int;

<<__EntryPoint>>
function main() :mixed{
  var_dump(json_decode(HH\embed_type_decl<Foo>()));
  var_dump(json_decode(HH\embed_type_decl<Bar>()));
  var_dump(json_decode(HH\embed_type_decl<DoesNotExist>()));
  var_dump(json_decode(HH\embed_type_decl<stdClass>()));
  var_dump(json_decode(HH\embed_type_decl<SoapHeader>()));
  var_dump(json_decode(HH\embed_type_decl<HH\Lib\Ref>()));
}
