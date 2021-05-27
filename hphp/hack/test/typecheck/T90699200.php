<?hh

<<__EntryPoint>>
function ko(): void {
  echo shape('key' => 42);

  echo true;

  echo (1 === 2 ? "hi" : null);

  echo new Exception();
}

function ok(): void {
  echo shape('key' => 42)['key'];

  echo (string) true;

  echo ((1 === 2 ? "hi" : null) as nonnull);

  echo (new Exception())->getMessage();
}
