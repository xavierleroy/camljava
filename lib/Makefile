include ../Makefile.config

OCAMLFIND=ocamlfind
OCAMLC=$(OCAMLFIND) ocamlc -g
OCAMLOPT=$(OCAMLFIND) ocamlopt
OCAMLDEP=$(OCAMLFIND) ocamldep
OCAMLLIB=`$(OCAMLC) -where`
CAMLJAVALIB=$(OCAMLLIB)/camljava

JNICFLAGS=-ccopt "$(JNILIBOPTS)" -cclib "$(JNILIBS)"

JAVA_FILES=$(wildcard fr/inria/caml/camljava/*.java)
# Incomplete list of .class files
JAVA_CLASSES=$(JAVA_FILES:.java=.class)

all: jni.cma jni.cmxa camljava.jar
byte: jni.cma camljava.jar

install:
	mkdir -p $(CAMLJAVALIB)
	cp jni.cma jni.cmi camljava.jar $(wildcard jni.cmxa jni.a) libcamljni.a jni.mli $(CAMLJAVALIB)

camljava.jar: $(JAVA_CLASSES)
	jar cf $@ fr/inria/caml/camljava/*.class

clean::
	rm -f camljava.jar

jni.cma: jni.cmo libcamljni.a
	$(OCAMLC) -linkall -a -o jni.cma -custom jni.cmo $(JNICFLAGS) -cclib -lcamljni

jni.cmxa: jni.cmx libcamljni.a
	$(OCAMLOPT) -linkall -a -o jni.cmxa jni.cmx $(JNICFLAGS) -cclib -lcamljni

libcamljni.a: jnistubs.o
	rm -f libcamljni.a
	ar rcs libcamljni.a jnistubs.o

clean::
	rm -f libcamljni.a

jni.ml: jni.mlp jni.mli ../Makefile.config
	rm -f jni.ml
	sed -e 's|%PATH%|'$(CAMLJAVALIB)/camljava.jar'|' \
                jni.mlp > jni.ml
	chmod -w jni.ml

clean::
	rm -f jni.ml

beforedepend:: jni.ml

$(JAVA_CLASSES): $(JAVA_FILES)
	$(JAVAC) $(JAVACFLAGS) $(JAVA_FILES)

clean::
	rm -f fr/inria/caml/camljava/*.class

clean::
	rm -f *.cm? *.[oa] *.cmxa

.SUFFIXES: .ml .mli .cmo .cmi .cmx

.c.o:
	$(OCAMLOPT) -c -ccopt "$(CFLAGS) $(JNIINCLUDES)" $*.c

.ml.cmo:
	$(OCAMLC) -c $*.ml

.ml.cmx:
	$(OCAMLOPT) -c $*.ml

.mli.cmi:
	$(OCAMLC) -c $*.mli

depend: beforedepend
	$(OCAMLDEP) *.mli *.ml > .depend

include .depend

