include ../Makefile.config

CAMLJAVA_PATH=../lib/camljava.jar
CAMLJAVA_DIR=../lib

#CAMLJAVA_PATH=`ocamlc -where`/camljava/camljava.jar
#CAMLJAVA_DIR=+camljava

all: jnitest Test.class Testcb.class
	CLASSPATH=$(CAMLJAVA_PATH):. ./jnitest

jnitest: jnitest.ml
	ocamlc -ccopt -g -o jnitest -I $(CAMLJAVA_DIR) jni.cma jnitest.ml

clean::
	rm -f jnitest

.SUFFIXES: .java .class

.java.class:
	$(JAVAC) -classpath $(CAMLJAVA_PATH):. $*.java

clean::
	rm -f *.cm?
	rm -f *.class
