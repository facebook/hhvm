<?hh

@ThisIsAnAttribute
@SoIsThis("ABC")
function foo() {}

<<__EntryPoint>>
function main() {
  $decode = ($text) ==> {
    $arr = json_decode($text, true);
    $decls = $arr['parse_tree']['script_declarations']['elements'];
    $ret = darray[];
    foreach ($decls as $elt) {
      if ($elt['kind'] !== 'function_declaration') continue;
      $attrs = $elt['function_attribute_spec']
                   ['attribute_specification_attributes']
                   ['elements'] ?? darray[];
      $name = $elt['function_declaration_header']
                  ['function_name']
                  ['token']
                  ['text'] ?? "";
      $ret[$name] = count($attrs);
    }
    return $ret;
  };
  var_dump(new ReflectionFunction("foo")->getAttributes());
  try {
    var_dump(hh\facts_parse("/",varray[__FILE__],true,false)[__FILE__]['functions']);
  } catch (InvalidOperationException $e) {}
  var_dump($decode(hh\ffp_parse_string_native(file_get_contents(__FILE__))));
}
