<?hh

<<__EntryPoint>>
function main() :mixed{
  try {
    // Serializable format
    var_dump(unserialize('C:1:"Y":3:{lol}'));
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
}
