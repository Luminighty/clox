#include "compiler.h"
#include "scanner.h"



void compile(const char *source) {
	scanner_init(source);

	Token token = scan_token();
	while (token.type != TOKEN_EOF) {

	}
}
