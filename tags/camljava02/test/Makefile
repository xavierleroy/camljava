include ../Makefile.config

CAMLJAVA=`ocamlc -where`/camljava/camljava.jar

all: jnitest Test.class Testcb.class
	./jnitest

jnitest: jnitest.ml
	ocamlc -ccopt -g -o jnitest -I +camljava jni.cma jnitest.ml

clean::
	rm -f jnitest

.SUFFIXES: .java .class

.java.class:
	$(JAVAC) -classpath $(CAMLJAVA):. $*.java

clean::
	rm -f *.cm?
