<?hh


<<__EntryPoint>>
async function main() :Awaitable<mixed>{
  $fname = sys_get_temp_dir().'/'.bin2hex(random_bytes(8));
  $f = fopen($fname, 'x+');
  try {
    fwrite($f, "foo");
    rewind($f);
    switch(php_uname('s')) {
      case 'Darwin':
        print 'MacOS: ';
        await \stream_await($f, STREAM_AWAIT_READ);
        print "OK\n";
        return;
      case 'Linux':
        print 'Linux: ';
        try {
          await \stream_await($f, STREAM_AWAIT_READ);
        } catch (InvalidOperationException $e) {
          print "OK\n";
          return;
        }
        print "FAILED!\n";
        return;
      default:
        print php_uname('s').": UNSUPPORTED\n";
        return;
    }
  } finally {
    fclose($f);
    unlink($fname);
  }
}
