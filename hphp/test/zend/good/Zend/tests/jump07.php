<?hh <<__EntryPoint>> function main(): void {
while (0) {
    L1: echo "allow\n";
    goto L2;
}
goto L1;
L2: return 0;
}
