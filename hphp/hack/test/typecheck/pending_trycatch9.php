<?hh

function test(): void {
  try {
    try {
    } catch (Exception $_) {
    }
    echo $x;
    $x = 1;
  } catch (Exception $_) {
    $x = 2;
  }
}
