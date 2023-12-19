<?hh

function data() {
	$tmp = pow(2,24);
	return dict[
		0=>'PHP',
		17=>'PHP: Hypertext Preprocessor',
		5=>'Test',
		'test'=>27,
		1000=>'test',
		"-1000"=>vec['banana', 'orange'],
		1001=>'monkey',
		$tmp=>-1/3
	];
}
