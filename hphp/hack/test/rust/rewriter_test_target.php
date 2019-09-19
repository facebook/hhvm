<?hh

<<__EntryPoint>>
function main_0() {
  $items = vec[];
  $items[] = 'Hello';
  $items[] = 'world!';
  if ($items[0] !== $items[1]) {
    print($items[0].', '.$items[1]);
  } else {
    print('duplicate pieces');
  }
}
