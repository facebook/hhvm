<?hh
<<__EntryPoint>> function main(): void {
error_reporting(0);

$a = vec[10, 20, 30];
print $a[0] . "\n";
print $a[1] . "\n";
print $a[2] . "\n";
try { print $a[3]; } catch (Exception $e) { echo $e->getMessage()."\n"; }
try { print $a["0"]; } catch (Exception $e) { echo $e->getMessage()."\n"; }
try { print $a["x"]; } catch (Exception $e) { echo $e->getMessage()."\n"; }

$s = "abc";
print $s[0] . "\n";
print $s[1] . "\n";
print $s[2] . "\n";
try { print $s[3]; } catch (Exception $e) { echo $e->getMessage()."\n"; }
print $s["0"] . "\n";
print $s["x"] . "\n";
}
