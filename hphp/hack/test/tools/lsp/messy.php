<?hh  //strict

function x(): string {
/* @lint-ignore TXT2 3 tabs on purpose */
			$a = "this";

/* @lint-ignore TXT2 2 tabs on purpose */
		$b = "is";

/* lint-ignore TXT2 1 tab on purpose */
	$c = "messy";  // 1 tab

    $d = ".";  // 4 spaces
return "$a" . "$b" . "$c" . "d";
}
