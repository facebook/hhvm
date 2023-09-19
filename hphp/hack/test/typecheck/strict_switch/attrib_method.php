<?hh

<<file: __EnableUnstableFeatures('strict_switch')>>

class A {
    <<__StrictSwitch>>
    private function mixed(mixed $x): void {
        switch($x) {
            default:
                break;
        }
    }
}
