<?hh <<__EntryPoint>> function main(): void {
$i=0;
$test2=darray[
   'a1_teasermenu' => darray[
           'downloadcounter' => 2777,
        'versions' => darray[
            '0.1.0' => darray [
                'title' => 'A1 Teasermenu',
                'description' => 'Displays a teaser for advanced subpages or a selection of advanced pages',
                'state' => 'stable',
                'reviewstate' => 0,
                'category' => 'plugin',
                'downloadcounter' => 2787,
                'lastuploaddate' => 1088427240,
                'dependencies' => darray [
                      'depends' => darray[
                              'typo3' =>'',
                              'php' =>'',
                              'cms' => ''
                       ],
                      'conflicts' => darray['' =>'']
                ],
                  'authorname' => 'Mirko Balluff',
                  'authoremail' => 'balluff@amt1.de',
                  'ownerusername' => 'amt1',
                  't3xfilemd5' => '3a4ec198b6ea8d0bc2d69d9b7400398f',
              ]
          ]
      ]
];
$test=varray[];
while($i<1200) {
    $test[]=$test2;
    $i++;
}
$out=serialize($test);
echo "ok\n";
}
