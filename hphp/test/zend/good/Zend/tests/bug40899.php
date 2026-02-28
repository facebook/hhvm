<?hh <<__EntryPoint>> function main(): void {
list(list($a,$b),$c)=vec[vec['a','b'],'c'];
echo "$a$b$c\n";
}
