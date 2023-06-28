<?hh

function ut_main()
:mixed{
        setlocale(LC_ALL, 'de_DE');
        $fmt = new NumberFormatter( 'sl_SI.UTF-8', NumberFormatter::DECIMAL);
        $num = "1.234.567,891";
        $res_str =  (string)($fmt->parse($num))."\n";
        $res_str .=  setlocale(LC_NUMERIC, 0);
        return $res_str;
}

<<__EntryPoint>> function main_entry(): void {
        include_once( 'ut_common.inc' );
        ut_run();
}
