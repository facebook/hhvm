
help:
	@echo "'make daemon' to start up doc server daemon"
	@echo "'make server' to start up doc server"
	@echo "'make clobber' to clean up directory"

daemon:
	sudo ../src/hhvm/hhvm -m daemon -v "Server.DefaultDocument=index.php" -v "Server.SourceRoot=`pwd`"

server:
	sudo ../src/hhvm/hhvm -m server -v "Server.DefaultDocument=index.php"

DEBUGGER_CMDS = a b c d e f g h i j k l m n o p q r step thread u v w x y z "\&" "!"

# update debugger docs
debugger:
	@../src/hhvm/hhvm -m debug --debug-cmd "help start" > debugger.start
	@../src/hhvm/hhvm -m debug --debug-cmd "help" > debugger.cmds
	@../src/hhvm/hhvm -m debug \
	$(patsubst %, --debug-cmd "'h %'", $(DEBUGGER_CMDS)) > debugger.refs

clean: clobber
clobber:
	@rm -f *~
