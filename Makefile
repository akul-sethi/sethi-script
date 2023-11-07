sethi: chunk.c chunk.h common.h compiler.c compiler.h debug.c debug.h main.c memory.c memory.h scanner.c scanner.h table.c table.h value.c value.h vm.c vm.h
	gcc chunk.c compiler.c debug.c main.c memory.c scanner.c table.c value.c vm.c -o sethi
	./sethi

no_run: chunk.c chunk.h common.h compiler.c compiler.h debug.c debug.h main.c memory.c memory.h scanner.c scanner.h table.c table.h value.c value.h vm.c vm.h
	gcc chunk.c compiler.c debug.c main.c memory.c scanner.c table.c value.c vm.c -o sethi

debug: chunk.c chunk.h common.h compiler.c compiler.h debug.c debug.h main.c memory.c memory.h scanner.c scanner.h table.c table.h value.c value.h vm.c vm.h
	gcc -g chunk.c compiler.c debug.c main.c memory.c scanner.c table.c value.c vm.c -o sethi


table_tests: chunk.c chunk.h common.h compiler.c compiler.h debug.c debug.h tests/table_tests.c memory.c memory.h scanner.c scanner.h table.c table.h value.c value.h vm.c vm.h
	gcc -g chunk.c compiler.c debug.c tests/table_tests.c memory.c scanner.c table.c value.c vm.c -o table_test


value_tests: chunk.c chunk.h common.h compiler.c compiler.h debug.c debug.h tests/value_tests.c memory.c memory.h scanner.c scanner.h table.c table.h value.c value.h vm.c vm.h
	gcc -g chunk.c compiler.c debug.c tests/value_tests.c memory.c scanner.c table.c value.c vm.c -o value_tests