<?php

if (!array_key_exists(1, $argv)) {
    exit("Specify markdown file you want to split as argument to split.php\n");
}

$data = file($argv[1]);
$filename = null;
$counter = 0;
$split_data = [];
$counts = [];
$filerefs = [];
$filecounts = [];

foreach($data as $line) {
    if($line[0] == '#') {
        // main header
        if($line[1] != '#') {
            $filename = sprintf("%02d-%s.md", $counter, strtolower(str_replace(" ", "-", trim(substr($line, 1)))));
            echo $filename, "\n";
            $counter++;
            $split_data[$filename] = '';
        }
        // sub-header
        $refname = trim(ltrim($line, '#'));
        $refname = str_replace(" ", "-", $refname);
        $refname = strtolower(preg_replace("/[^-_A-Za-z0-9]/", "", $refname));
        if(!empty($references[$refname])) {
            @$counts[$refname]++;
            $orig_refname = $refname;
            $refname .= "-".$counts[$refname];
            // Local references for repeating names may be different from global ones
            if(!isset($filecounts[$filename][$orig_refname])) {
                $filecounts[$filename][$orig_refname] = 0;
                $filerefs[$filename][$refname] = $orig_refname;
            } else {
                @$filecounts[$filename][$orig_refname]++;
                $filerefs[$filename][$refname] = $orig_refname . "-" . $filecounts[$filename][$orig_refname];
            }
        }
        $references[$refname] = $filename;
    }
    $split_data[$filename] .= $line;
}

foreach($split_data as $filename => $contents) {
    $contents = preg_replace_callback('@\[(.*?)\]\(#(.*?)\)@', function($data) use($references, $filename, $filerefs) {
        if(empty($references[$data[2]])) {
            return $data[0];
        }
        if($references[$data[2]]) {
            $filepart = $references[$data[2]];
        } else {
            $filepart = $filename;
        }
        if(!empty($filerefs[$filepart][$data[2]])) {
            // replace global ref with local one
            $refname = $filerefs[$filepart][$data[2]];
        } else {
            $refname = $data[2];
        }
        if($filepart == $filename) {
            $filepart = "";
        }
        $res = sprintf("[%s](%s#%s)", $data[1], $filepart, $refname);
        return $res;
    }, $contents);
    file_put_contents($filename, $contents);
}
