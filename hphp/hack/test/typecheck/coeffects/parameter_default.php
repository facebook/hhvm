<?hh

<<file: __EnableUnstableFeatures('coeffects_provisional')>>

function impure()[int]: int { return 4; }

function pure(int $def = impure())[]: void {}
