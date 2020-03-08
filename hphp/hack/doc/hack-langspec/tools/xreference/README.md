This tool tries to reconstruct the internal cross references that existed
in the Word document that was used to originally create the draft spec.

If we are lucky, this will only need to be run once as we convert from .docx
to .md.

We first manually grab all of the section numbers from the Word file and
paste them into a CSV-aware file (like an Excel file). 

**BE CAREFUL** to get all the section numbers. The Word ToC normally only
goes up to 5 levels. But the document itself could have levels more than 
that; so you may have to do a manual search.

Now we have a column of section numbers.

Then we run xreference.php. This does the following:

1. Goes through our Table of Contents and maps, in line order, the current
GitHub link anchor to the section number in our CSV file. This is fragile
because it assumes that the section numbers in the CSV file map 1:1 in
location to that in the ToC.

2. Then it uses our final CSV mapping to replace the Word section numbers
with the numbers and GitHub anchor links.

`§11.7.5 => §11_7_5(#the-return-statement)`

3. Optionally, the numbers text can be changed to a constant character string

`§11.7.5 => §§(#the-return-statement)`


Here is an example on how to run it:

```
hhvm xreference.php -i ../../spec/00-specification-for-hack.md -m ../../spec/ -s section_map.csv -t §§
```


**NOTE** if you run this tool and see HHVM Notices like:

> Notice: Undefined index: 10.17.2 in hack-langspec/tools/xreference/xreference.php on line 46

That means the your markdown has in it a reference that doesn't exist in the section map, so something is wrong in one of those two files.
