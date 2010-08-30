
help:
	@echo "'make daemon' to start up doc server daemon"
	@echo "'make server' to start up doc server"
	@echo "'make clobber' to clean up directory"

daemon:
	sudo ../src/hphpi/hphpi -m daemon -v "Server.DefaultDocument=index.php" -v "Server.SourceRoot=`pwd`"

server:
	sudo ../src/hphpi/hphpi -m server -v "Server.DefaultDocument=index.php"

DEBUGGER_CMDS = a b c d e f g h i j k l m n o p q r step thread u v w x y z "\&" "!"

# update debugger docs
debugger:
	@../src/hphpi/hphpi -m debug --debug-cmd "help start" > debugger.start
	@../src/hphpi/hphpi -m debug --debug-cmd "help" > debugger.cmds
	@../src/hphpi/hphpi -m debug \
	$(patsubst %, --debug-cmd "'h %'", $(DEBUGGER_CMDS)) > debugger.refs

clobber:
	@rm -f *~
