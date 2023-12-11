<?hh <<__EntryPoint>> function main(): void {
$i=0;
$test2=dict[
   'a1_teasermenu' => dict[
           'downloadcounter' => 2777,
        'versions' => dict[
            '0.1.0' => darray [
                'title' => 'A1 Teasermenu',
                'description' => 'Displays a teaser for advanced subpages or a selection of advanced pages',
                'state' => 'stable',
                'reviewstate' => 0,
                'category' => 'plugin',
                'downloadcounter' => 2787,
                'lastuploaddate' => 1088427240,
                'dependencies' => darray [
                      'depends' => dict[
                              'typo3' =>'',
                              'php' =>'',
                              'cms' => ''
                       ],
                      'conflicts' => dict['' =>'']
                ],
                  'authorname' => 'Mirko Balluff',
                  'authoremail' => 'balluff@amt1.de',
                  'ownerusername' => 'amt1',
                  't3xfilemd5' => '3a4ec198b6ea8d0bc2d69d9b7400398f',
              ]
          ]
      ]
];
$test=vec[];
while($i<1200) {
    $test[]=$test2;
    $i++;
}
$out=serialize($test);
echo "ok\n";
}
