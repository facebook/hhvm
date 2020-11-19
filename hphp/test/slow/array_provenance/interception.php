<?hh

<<__NEVER_INLINE>>
function handler($event, $name, $info) {
  if ($name === 'fb_setprofile') return;
  print("$event $name: ".'$info: '.HH\get_provenance($info)."\n");
  if ($event === 'enter') {
    print("$event $name: ".'$info["args"]: '.HH\get_provenance($info['args'])."\n");
    foreach ($info['args'] as $i => $arg) {
      print("$event $name: ".'$info["args"]['.$i.']: '.HH\get_provenance($arg)."\n");
    }
  }
}

<<__NEVER_INLINE>>
function test($a) {
  return 17;
}

<<__EntryPoint>>
function main() {
  fb_setprofile(handler<>);
  test(varray[]);
  test(vec[]);
}
