<?hh

function takes_string(string $_): void {}

function call_it(int $i): void {
  takes_string(HH\FIXME\UNSAFE_CAST<int,string>($i, 'lying to hh is bad'));
  //                      ^ hover-at-caret
}
