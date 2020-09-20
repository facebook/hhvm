<?hh

<<__EntryPoint>>
function main() {
  var_dump(json_decode("{\"1\": 1, \"2\": \"2\"}", true, 512, JSON_FB_DARRAYS));
}
