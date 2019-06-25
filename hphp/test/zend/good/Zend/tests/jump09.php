<?hh <<__EntryPoint>> function main(): void {
switch (0) {
    case 1:
        L1: echo "allow\n";
        goto L2;
        break;
}
goto L1;
L2: return 0;
}
