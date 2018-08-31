<?hh


<<__EntryPoint>>
function main_ephemeral() {
(new IntervalTimer(1, 1, () ==> {}))->start();
echo "OK\n";
}
