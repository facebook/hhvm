<?hh

enum Color: string {
  RED = 'red';
  BLUE = 'blue';
}

function takes_color(Color $c): void {}

function test(): void {
  $x = Color::RED;
  takes_color($x);
//            ^ enforcement-at-caret
}
