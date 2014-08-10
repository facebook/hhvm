<?hh

function f(): AsyncGenerator<string, int, void> {
  $g = async function() {
    yield 'zero' => 0;
  };

  return $g();
}
