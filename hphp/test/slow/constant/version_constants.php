<?hh // strict

<<__EntryPoint>>
function main() {
  // The purpose of this test is to make sure that HHVM_VERSION_MAJOR, _MINOR,
  // and _PATCH are defined; it is not intended to verify the format of
  // HHVM_VERSION_ID - please update this test if the format of HHVM_VERSION_ID
  // changes.
  $id =
    HHVM_VERSION_PATCH +
    HHVM_VERSION_MINOR * 100 +
    HHVM_VERSION_MAJOR * 10000;
  var_dump($id === HHVM_VERSION_ID);
}
