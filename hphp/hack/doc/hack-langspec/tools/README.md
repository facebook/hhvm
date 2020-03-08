Tools and scripts to help with rendering and enhancing the documentation in general.

Here are some possible examples:

1. A script to convert from Markdown to Word or PDF might live in here.
2. A tool to help move cross reference links from Word to Markdown.
3. A script to add numbered headings to Markdown.

If you are converting from something like Word to Markdown for the first time, this the order of tool running you might consider:

1. If the documentation has been converted from Word to **one big** markdown file, you will probably want to run `split.php` first. This will split the big markdown into multiple markdown files based on primary section headings. If the document is already split, then you can go to step (2).
2. Then you will want to run `toc.php` to create the table of contents for the split documentation.
3. Then run `xreference.php` to get all the cross references correct. For documentation that has been split into multiple markdown files, you can specify a directory of those files so that all the files there get modified for cross-references. If the documentation is one big file still, you can just specify that file.

