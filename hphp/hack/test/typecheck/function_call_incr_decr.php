<?hh

function get_dict(): dict<string, int> {
  return dict[];
}

function foobar(): void {
  get_dict()['lol']--;
  get_dict()['hah']++;
}
