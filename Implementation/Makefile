all: servermake clientmake test1make test2make

servermake: src/server/*.c src/server/Game/*.c src/general/SerializeDeserialize/*.c src/general/LinkedList/*.c src/general/*.c
	gcc -o server src/server/*.c src/server/Game/*.c src/general/SerializeDeserialize/*.c src/general/LinkedList/*.c src/general/*.c -lncurses -lpthread -I.
	echo Server successfully compiled.

clientmake: src/client/*.c src/client/Game/*.c src/general/SerializeDeserialize/*.c src/general/LinkedList/*.c src/general/*.c
	gcc -o client src/client/*.c src/client/Game/*.c src/general/SerializeDeserialize/*.c src/general/LinkedList/*.c src/general/*.c -lncurses -lpthread -I.
	echo Client successfully compiled.
	
test1make: src/server/Game/Snake.c src/tests/LinkedListTest.c src/general/LinkedList/LinkedList.c
	gcc -o test1 src/server/Game/Snake.c src/tests/LinkedListTest.c src/general/LinkedList/LinkedList.c -I.
	echo Test 1 successfully compiled.

test2make: src/general/General.c src/tests/SerAndDesTest.c src/general/SerializeDeserialize/*.c
	gcc -o test2 src/general/General.c src/tests/SerAndDesTest.c src/general/SerializeDeserialize/*.c -I.
	echo Test 2 successfully compiled.
