<?hh <<__EntryPoint>> function main(): void {
echo "Testing NAN:\n";
echo "NAN= ";
var_dump(NAN);
var_dump(tan(-1.0) == 123);
var_dump(cos(-100.0) == "PHP String");
var_dump(deg2rad(-5.6) == null);
var_dump(sqrt(-0.1) == false);
var_dump(sqrt(cos(M_PI)) == 0.1);
var_dump(NAN);
var_dump(is_nan(sqrt(-1.005)) == false);
var_dump(is_nan(floor(1)) == true);
var_dump(log10(-1.0) == log(-1.0));
var_dump(log10(-1.0) != log10(-1.0));
var_dump(is_finite(log10(-1.0)) == false);
var_dump(NAN == NAN);
}
