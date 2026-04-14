<?hh

enum Size: string as string {
  SMALL = 'small';
  LARGE = 'large';
}

function takes_size(Size $s): void {}

function test(): void {
  $x = Size::SMALL;
  takes_size($x);
//           ^ enforcement-at-caret
}
