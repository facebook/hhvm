<?hh
function do_translit($st) :mixed{
    $replacement = dict[
        "\xd0\xb9"=>"i","\xd1\x86"=>"c","\xd1\x83"=>"u","\xd0\xba"=>"k","\xd0\xb5"=>"e","\xd0\xbd"=>"n",
        "\xd0\xb3"=>"g","\xd1\x88"=>"sh","\xd1\x89"=>"sh","\xd0\xb7"=>"z","\xd1\x85"=>"x","\xd1\x8a"=>"\'",
        "\xd1\x84"=>"f","\xd1\x8b"=>"i","\xd0\xb2"=>"v","\xd0\xb0"=>"a","\xd0\xbf"=>"p","\xd1\x80"=>"r",
        "\xd0\xbe"=>"o","\xd0\xbb"=>"l","\xd0\xb4"=>"d","\xd0\xb6"=>"zh","\xd1\x8d"=>"ie","\xd1\x91"=>"e",
        "\xd1\x8f"=>"ya","\xd1\x87"=>"ch","\xd1\x81"=>"c","\xd0\xbc"=>"m","\xd0\xb8"=>"i","\xd1\x82"=>"t",
        "\xd1\x8c"=>"\'","\xd0\xb1"=>"b","\xd1\x8e"=>"yu",
        "\xd0\x99"=>"I","\xd0\xa6"=>"C","\xd0\xa3"=>"U","\xd0\x9a"=>"K","\xd0\x95"=>"E","\xd0\x9d"=>"N",
        "\xd0\x93"=>"G","\xd0\xa8"=>"SH","\xd0\xa9"=>"SH","\xd0\x97"=>"Z","\xd0\xa5"=>"X","\xd0\xaa"=>"\'",
        "\xd0\xa4"=>"F","\xd0\xab"=>"I","\xd0\x92"=>"V","\xd0\x90"=>"A","\xd0\x9f"=>"P","\xd0\xa0"=>"R",
        "\xd0\x9e"=>"O","\xd0\x9b"=>"L","\xd0\x94"=>"D","\xd0\x96"=>"ZH","\xd0\xad"=>"IE","\xd0\x81"=>"E",
        "\xd0\xaf"=>"YA","\xd0\xa7"=>"CH","\xd0\xa1"=>"C","\xd0\x9c"=>"M","\xd0\x98"=>"I","\xd0\xa2"=>"T",
        "\xd0\xac"=>"\'","\xd0\x91"=>"B","\xd0\xae"=>"YU",
    ];
   
    foreach($replacement as $i=>$u) {
        $st = mb_eregi_replace($i,$u,$st);
    }
    return $st;
}
<<__EntryPoint>>
function main_entry(): void {

  mb_regex_encoding('ISO-8859-1');
  echo do_translit("\xd0\x9f\xd0\xb5\xd0\xb0\xd1\x80");
}
