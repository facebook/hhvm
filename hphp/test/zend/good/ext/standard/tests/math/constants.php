<?hh
<<__EntryPoint>> function main(): void {
$constants = vec[
    "M_E",
    "M_LOG2E",
    "M_LOG10E",
    "M_LN2",
    "M_LN10",
    "M_PI",
    "M_PI_2",
    "M_PI_4",
    "M_1_PI",
    "M_2_PI",
    "M_SQRTPI",
    "M_2_SQRTPI",
    "M_LNPI",
    "M_EULER",
    "M_SQRT2",
    "M_SQRT1_2",
    "M_SQRT3"
];
foreach($constants as $constant) {
    printf("%-10s: %s\n", $constant, constant($constant));
}
}
