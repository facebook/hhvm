<?hh


<<__EntryPoint>>
function main_ephemeral() {
(new IntervalTimer(1.0, 1.0, () ==> {}))->start();
echo "OK\n";
}
