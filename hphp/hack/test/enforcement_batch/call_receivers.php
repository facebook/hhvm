<?hh

function top_level_takes_int(int $_x): void {}

function test_top_level_function(): void {
  top_level_takes_int(42);
//                    ^ enforcement-at-caret
}
