<?hh

class something {}
type blah = ?something;
function bar2(blah $k) {}
bar2("fail");
