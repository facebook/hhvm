On a devserver:

Generate the HHAS text file
$ /usr/local/hphpi/bin/hhvm -v Eval.DumpHhas=1 <your_php_file> > <your_hhas_file>

Execute the HHAS text filename
$ /usr/local/hphpi/bin/hhvm -v Eval.AllowHhas=1 <your_hhas_file>

Generate the SQlite repo, called the repository
$ /usr/local/hphpi/bin/hphp --target hhbc -v Runtime.Eval.AllowHhas=1 <your_hhas_file>
--> it gives a path to a repo e.g. /tmp/hphp_somehash/hhvm.hhbbc

Run it
$ /usr/local/hphpi/bin/hhvm -v Repo.Authoritative=1 -v Repo.Central.Path=<your_hhbc_repo_file> <your_php_file>
