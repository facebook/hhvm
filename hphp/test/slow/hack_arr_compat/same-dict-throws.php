<?hh

function d(dict $a, dict $d) {
  return $a === $d;
}

function v(vec $a, vec $d) {
  return $a === $d;
}

const a = 'a';
<<__EntryPoint>>
function main_entry(): void {

  set_error_handler(function() { throw new Exception; });

  for ($i = 0; $i < 10; $i++) {
    try {
      d(dict[a => dict[]], dict[a => array()]);
    } catch (Exception $e) {
      echo ".";
    }
    try {
      v(vec[dict[]], vec[array()]);
    } catch (Exception $e) {
      echo ".";
    }
  }
}
