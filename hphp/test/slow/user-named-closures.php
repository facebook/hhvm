<?hh

/* This shouldn't work from user PHP code, so this should output false */

function get_exn_frame_name($f) {
  try {
    return $f();
  } catch (Exception $e) {
    return $e->getTrace()[0]["function"];
  }
}

<<__EntryPoint>>
function main() {
  var_dump(false !== strpos(get_exn_frame_name(
    <<__ClosureName("hello")>>
    function () {
      throw new Exception();
    }), "hello"));
  var_dump(false !== strpos(get_exn_frame_name(
    <<__ClosureName("goodbye")>>
    function () {
      throw new Exception();
    }), "goodbye"));
}
