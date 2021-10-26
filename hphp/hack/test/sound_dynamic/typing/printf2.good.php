<?hh

interface Dummy {
  public function format_s(~string $s) : ~string;
}

function takesFS(~?\HH\FormatString<Dummy> $f = null) : void {}

function _() : void {
  takesFS();
  takesFS(null);
  takesFS('%s', 's');
}
