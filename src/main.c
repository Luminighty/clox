#include <stdio.h>

#include "file.h"
#include "repl.h"
#include "vm.h"

int main(int argc, char* argv[]) {

	vm_create();

	if (argc == 1) {
		repl();
	} else if (argc == 2) {
		file_run(argv[1]);
	} else {
		fprintf(stderr, "Usage: clox [path]\n");
	}

	return 0;
}

