include ../Makefile.config

CAMLJAVA=`ocamlc -where`/camljava/camljava.jar

all: jnitest Test.class Testcb.class
	./jnitest

jnitest: jnitest.ml
	ocamlc -ccopt -g -o jnitest -I +camljava jni.cma jnitest.ml

.SUFFIXES: .java .class

.java.class:
	$(JAVAC) -classpath $(CAMLJAVA) $*.java
