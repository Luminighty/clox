#include "repl.h"
#include "vm.h"

#include <stdio.h>


void repl() {

	char line[1024];
	for(;;) {
		printf("> ");

		if (!fgets(line, sizeof(line), stdin)) {
			printf("\n");
			break;
		}

		vm_interpret(line);
	}

}

