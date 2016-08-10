<?hh
function f () {
  $a = async {};
  $a = async {} + 1;
  $a = 1 + async {};
  $a = async {} + async {};
  $a = async {
    $a = 1;
  };
  $a = async {
    $b = async {} + 1;
  };
}
