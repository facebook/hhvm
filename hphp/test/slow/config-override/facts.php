<?hh

@ThisIsAnAttribute
@SoIsThis("ABC")
function foo() {}

<<__EntryPoint>>
function main() {
  $decode = ($text) ==> {
    $arr = json_decode($text, true);
    $decls = $arr['parse_tree']['script_declarations']['elements'];
    $ret = dict[];
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

  $attrs = dict[];
  foreach (new ReflectionFunction("foo")->getAttributes() as $name => $attr) {
    $attrs[$name] = vec($attr);
  }
  var_dump($attrs);

  try {
    var_dump(HH\Facts\path_to_functions(__FILE__));
  } catch (InvalidOperationException $e) {}

  var_dump($decode(HH\ffp_parse_string_native(file_get_contents(__FILE__))));
}
