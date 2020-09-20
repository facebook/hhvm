<?hh

function test(): void {
  $a = vec[
        1, 2, 3, 4, 5
       ];

  // hackfmt-ignore - this should work
  $b = vec[
        2, 3, 4, 5, 6
       ];



       $c = vec[
             1, 2, 3, 4, 5
            ];

       // hackfmt-ignore - this should work
       $d = vec[
             2, 3, 4, 5, 6
           ]; // some comment that should not disappear

            $e = vec[
                  1, 2, 3, 4, 5
                 ];

            // hackfmt-ignore - this should work
            $f = vec[
                  2, 3, 4, 5, 6
                 ];
                 $g = vec[
                       1, 2, 3, 4, 5
                      ];
  $h = vec[
    // hackfmt-ignore - this should not work
    3, 4, 5, 6, 7
  ];

  $i = vec[
    4, 5, 6, 7, 8
  ];

  /*
    hackfmt-ignore - this multiline comment should also work
  */
  $j = vec[
    4, 5, 6, 7, 8
  ];

  $k = vec[
    5, 6, 7, 8, 9
  ];
}
