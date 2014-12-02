name = nonogram
uname = $(shell echo $(name) | tr a-z A-Z)
version = $(shell head -n1 changelog | cut -d ' ' -f2 | tr -d '()')

.PHONY: all
all: $(name).1

%.1: manpage.pod changelog
	pod2man --utf8 -c '' -n $(uname) -r '$(name) $(version)' $(<) \
	> $(@).tmp
	mv $(@).tmp $(@)

.PHONY: clean
clean:
	rm -f *.1 *.tmp

# vim:ts=4 sts=4 sw=4 noet
