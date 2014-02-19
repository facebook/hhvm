<?PHP

var_dump(parse_ini_string("
[ a = c ]
b = c
d[] = e
d[  ] = f
d[ 'g '  ] = h
i[1][2][34] = j
i[1][2][5][6] = k
"));
