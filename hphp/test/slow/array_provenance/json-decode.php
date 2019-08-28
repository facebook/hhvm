<?hh

<<__EntryPoint>>
function main() {
  $json = "{\"x\":[{\"y\":1123990268},[4,2]]}";
  $data = json_decode($json, true, 512, JSON_FB_HACK_ARRAYS);

  var_dump(HH\get_provenance($data));
  var_dump(HH\get_provenance($data['x']));
  var_dump(HH\get_provenance($data['x'][0]));
  var_dump(HH\get_provenance($data['x'][1]));
  var_dump(HH\get_provenance($data['x'][0]['y']));
}
