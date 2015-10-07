<?php

function cleanPagePath($path)
{
	$path = strtolower($path);
	$tmp = '';
	$pathLength = strlen($path);
	for ($i = 0; $i < $pathLength; $i++)
	{
		switch ($path[$i])
		{
			case 'a':
			case 'b':
			case 'c':
			case 'd':
			case 'e':
			case 'f':
			case 'g':
			case 'h':
			case 'i':
			case 'j':
			case 'k':
			case 'l':
			case 'm':
			case 'n':
			case 'o':
			case 'p':
			case 'q':
			case 'r':
			case 's':
			case 't':
			case 'u':
			case 'v':
			case 'w':
			case 'x':
			case 'y':
			case 'z':
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
			case '_':
			case '-':
			case '/':
				$tmp .= $path[$i];
				continue;

			// Backslashes are looked up as forward slashes.
			case '\\':
				$tmp .= '/';
				continue;

			// Spaces are replaced with underscores.
			case ' ':
				$tmp .= '_';
				continue;

			default:
				continue;
		}
	}

	return $tmp;
}

echo cleanPagePath('ab2&13_-/ \\');