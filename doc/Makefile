
help:
	@echo "'make daemon' to start up doc server daemon"
	@echo "'make server' to start up doc server"
	@echo "'make clobber' to clean up directory"

daemon:
	sudo ../src/hphpi/hphpi -m daemon -v "Server.DefaultDocument=index.php" -v "Server.SourceRoot=`pwd`"

server:
	sudo ../src/hphpi/hphpi -m server -v "Server.DefaultDocument=index.php"

clobber:
	@rm -f *~
