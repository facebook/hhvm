OTT=ott
OTT_OPTIONS=-signal_parse_errors true

all: \
	hack_typing.ml \
	hack_typing.v \
	hack_typing.pdf

hack_typing.tex: hack_typing.ott
	$(OTT) $(OTT_OPTIONS) -i hack_typing.ott -o hack_typing.tex

hack_typing.ml: hack_typing.ott
	$(OTT) $(OTT_OPTIONS) -i hack_typing.ott -o hack_typing.ml

hack_typing.v: hack_typing.ott
	$(OTT) $(OTT_OPTIONS) -i hack_typing.ott -o hack_typing.v

hack_typing.pdf: hack_typing.tex
	pdflatex hack_typing.tex

clean:
	rm -rf *~
	rm -rf hack_typing.tex hack_typing.ml hack_typing.v hack_typing.pdf
	rm -rf hack_typing.aux hack_typing.log
