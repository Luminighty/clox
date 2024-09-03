#include "compiler.h"
#include "chunk.h"
#include "object.h"
#include "scanner.h"
#include "value.h"

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct {
	Token current;
	Token previous;
	bool had_error;
	bool panic_mode;
} Parser;

typedef enum {
	PREC_NONE,
	PREC_ASSIGNMENT,
	PREC_OR,
	PREC_AND,
	PREC_EQUALITY,
	PREC_COMPARISON,
	PREC_TERM,
	PREC_FACTOR,
	PREC_UNARY,
	PREC_CALL,
	PREC_PRIMARY
} Precedence;


typedef void (*ParseFn)(bool can_assign);

typedef struct {
	ParseFn prefix;
	ParseFn infix;
	Precedence precedence;
} ParseRule;


Parser parser;

Chunk* compiling_chunk;

static Chunk* current_chunk() {
	return compiling_chunk;
}


static void error_at(Token* token, const char* message) {
	if (parser.panic_mode) return;
	parser.panic_mode = true;
	fprintf(stderr, "[line %d] Error", token->line);

	if (token->type == TOKEN_EOF) {
		fprintf(stderr, " at end of file");
	} else if (token->type == TOKEN_ERROR) {

	} else {
		fprintf(stderr, " at '%.*s'", token->length, token->start);
	}
	fprintf(stderr, ": %s\n", message);
	parser.had_error = true;
}


static void error_at_current(const char* message) {
	error_at(&parser.current, message);
}


static void error(const char* message) {
	error_at(&parser.previous, message);
}


static void advance() {
	parser.previous = parser.current;

	for (;;) {
		parser.current = scan_token();
		if (parser.current.type != TOKEN_ERROR)
			break;
		error_at_current(parser.current.start);
	}
}


static void consume(TokenType type, const char* message) {
	if (parser.current.type == type) {
		advance();
		return;
	}

	error_at_current(message);
}


static bool check(TokenType type) {
	return parser.current.type == type;
}


static bool match(TokenType type) {
	if (!check(type)) return false;
	advance();
	return true;
}


static void emit_byte(uint8_t byte) {
	chunk_write(current_chunk(), byte, parser.previous.line);
}


static void emit_bytes(uint8_t byte_1, uint8_t byte_2) {
	emit_byte(byte_1);
	emit_byte(byte_2);
}


static void emit_return() {
	emit_byte(OP_RETURN);
}


static void compiler_end() {
	emit_return();
	#ifdef DEBUG_PRINT_CODE
	if (!parser.had_error)
		disassemble_chunk(current_chunk(), "code");
	#endif
}


static uint8_t make_constant(Value value) {
	int constant = chunk_write_constant(current_chunk(), value);
	if (constant > UINT8_MAX) {
		error("Too many constant in one chunk.");
		return 0;
	}

	return (uint8_t)constant;
}


static void emit_constant(Value value) {
	emit_bytes(OP_CONSTANT, make_constant(value));
}


static void synchronize() {
	parser.panic_mode = false;
	while (parser.current.type != TOKEN_EOF) {
		if (parser.previous.type == TOKEN_SEMICOLON)
			return;
		
		switch (parser.current.type) {
		case TOKEN_CLASS:
		case TOKEN_FUN:
		case TOKEN_VAR:
		case TOKEN_FOR:
		case TOKEN_IF:
		case TOKEN_WHILE:
		case TOKEN_PRINT:
		case TOKEN_RETURN:
			return;
		default:;
		}
		advance();
	}
}


static uint8_t identifier_constant(Token* name) {
	return make_constant(VALUE_OBJECT(string_copy(name->start, name->length)));
}

static uint8_t variable_parse(const char* error) {
	consume(TOKEN_IDENTIFIER, error);
	return identifier_constant(&parser.previous);
}

static void variable_define(uint8_t global) {
	emit_bytes(OP_DEFINE_GLOBAL, global);
}


static void parse_precedence(Precedence precedence);
static ParseRule* parse_rule_get(TokenType type);

static void expression();
static void statement();
static void declaration();


static void variable_named(Token name, bool can_assign) {
	uint8_t arg = identifier_constant(&name);

	if (can_assign && match(TOKEN_EQUAL)) {
		expression();
		emit_bytes(OP_SET_GLOBAL, arg);
	} else {
		emit_bytes(OP_GET_GLOBAL, arg);
	}
}

static void number(bool can_assign) {
	double value = strtod(parser.previous.start, NULL);
	emit_constant(VALUE_NUMBER(value));
}


static void string(bool can_assign) {
	emit_constant(VALUE_OBJECT(string_copy(parser.previous.start + 1, parser.previous.length - 2)));
}

static void variable(bool can_assign) {
	variable_named(parser.previous, can_assign);
}

static void statement_print() {
	expression();
	consume(TOKEN_SEMICOLON, "Expect ';' after value.");
	emit_byte(OP_PRINT);
}

static void expression() {
	parse_precedence(PREC_ASSIGNMENT);


}


static void declaration_var() {
	uint8_t global = variable_parse("Expect variable name");

	if (match(TOKEN_EQUAL)) {
		expression();
	} else {
		emit_byte(OP_NIL);
	} 
	consume(TOKEN_SEMICOLON, "Expect ';' after variable declaration.");
	
	variable_define(global);
}



static void declaration() {
	if (match(TOKEN_VAR)) {
		declaration_var();
	} else {
		statement();
	}

	if (parser.panic_mode)
		synchronize();
}

static void expression_statement() {
	expression();
	consume(TOKEN_SEMICOLON, "Expect ';' after expression.");
	emit_byte(OP_POP);
}

static void statement() {
	if (match(TOKEN_PRINT)) {
		statement_print();
	} else {
		expression_statement();
	}
}

static void grouping(bool can_assign) {
	expression();
	consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}


static void literal(bool can_assign) {
	switch (parser.previous.type) {
	case TOKEN_FALSE: emit_byte(OP_FALSE); break;
	case TOKEN_TRUE: emit_byte(OP_TRUE); break;
	case TOKEN_NIL: emit_byte(OP_NIL); break;
	default: return;
	}
}


static void unary(bool can_assign) {
	TokenType operator = parser.previous.type;

	parse_precedence(PREC_UNARY);

	switch (operator) {
	case TOKEN_MINUS: emit_byte(OP_NEGATE); break;
	case TOKEN_BANG: emit_byte(OP_NOT); break;
	default: return;
	}
}


static void binary(bool can_assign) {
	TokenType operator = parser.previous.type;
	ParseRule* rule = parse_rule_get(operator);
	parse_precedence((Precedence)(rule->precedence + 1));

	switch (operator) {
	case TOKEN_PLUS: emit_byte(OP_ADD); break;
	case TOKEN_MINUS: emit_byte(OP_SUBTRACT); break;
	case TOKEN_STAR: emit_byte(OP_MULTIPLY); break;
	case TOKEN_SLASH: emit_byte(OP_DIVIDE); break;
	case TOKEN_BANG_EQUAL: emit_bytes(OP_EQUAL, OP_NOT); break;
	case TOKEN_EQUAL_EQUAL: emit_byte(OP_EQUAL); break;
	case TOKEN_GREATER: emit_byte(OP_GREATER); break;
	case TOKEN_GREATER_EQUAL: emit_bytes(OP_LESS, OP_NOT); break;
	case TOKEN_LESS: emit_byte(OP_LESS); break;
	case TOKEN_LESS_EQUAL: emit_bytes(OP_GREATER, OP_NOT); break;
	default: return;
	}
}



ParseRule rules[] = {
	[TOKEN_LEFT_PAREN]    = {grouping, NULL,   PREC_NONE},
	[TOKEN_RIGHT_PAREN]   = {NULL,     NULL,   PREC_NONE},
	[TOKEN_LEFT_BRACE]    = {NULL,     NULL,   PREC_NONE}, 
	[TOKEN_RIGHT_BRACE]   = {NULL,     NULL,   PREC_NONE},
	[TOKEN_COMMA]         = {NULL,     NULL,   PREC_NONE},
	[TOKEN_DOT]           = {NULL,     NULL,   PREC_NONE},
	[TOKEN_MINUS]         = {unary,    binary, PREC_TERM},
	[TOKEN_PLUS]          = {NULL,     binary, PREC_TERM},
	[TOKEN_SEMICOLON]     = {NULL,     NULL,   PREC_NONE},
	[TOKEN_SLASH]         = {NULL,     binary, PREC_FACTOR},
	[TOKEN_STAR]          = {NULL,     binary, PREC_FACTOR},
	[TOKEN_BANG]          = {unary,    NULL,   PREC_NONE},
	[TOKEN_BANG_EQUAL]    = {NULL,     binary, PREC_EQUALITY},
	[TOKEN_EQUAL]         = {NULL,     NULL,   PREC_NONE},
	[TOKEN_EQUAL_EQUAL]   = {NULL,     binary, PREC_EQUALITY},
	[TOKEN_GREATER]       = {NULL,     binary, PREC_COMPARISON},
	[TOKEN_GREATER_EQUAL] = {NULL,     binary, PREC_COMPARISON},
	[TOKEN_LESS]          = {NULL,     binary, PREC_COMPARISON},
	[TOKEN_LESS_EQUAL]    = {NULL,     binary, PREC_COMPARISON},
	[TOKEN_IDENTIFIER]    = {variable, NULL,   PREC_NONE},
	[TOKEN_STRING]        = {string,   NULL,   PREC_NONE},
	[TOKEN_NUMBER]        = {number,   NULL,   PREC_NONE},
	[TOKEN_AND]           = {NULL,     NULL,   PREC_NONE},
	[TOKEN_CLASS]         = {NULL,     NULL,   PREC_NONE},
	[TOKEN_ELSE]          = {NULL,     NULL,   PREC_NONE},
	[TOKEN_FALSE]         = {literal,  NULL,   PREC_NONE},
	[TOKEN_FOR]           = {NULL,     NULL,   PREC_NONE},
	[TOKEN_FUN]           = {NULL,     NULL,   PREC_NONE},
	[TOKEN_IF]            = {NULL,     NULL,   PREC_NONE},
	[TOKEN_NIL]           = {literal,  NULL,   PREC_NONE},
	[TOKEN_OR]            = {NULL,     NULL,   PREC_NONE},
	[TOKEN_PRINT]         = {NULL,     NULL,   PREC_NONE},
	[TOKEN_RETURN]        = {NULL,     NULL,   PREC_NONE},
	[TOKEN_SUPER]         = {NULL,     NULL,   PREC_NONE},
	[TOKEN_THIS]          = {NULL,     NULL,   PREC_NONE},
	[TOKEN_TRUE]          = {literal,  NULL,   PREC_NONE},
	[TOKEN_VAR]           = {NULL,     NULL,   PREC_NONE},
	[TOKEN_WHILE]         = {NULL,     NULL,   PREC_NONE},
	[TOKEN_ERROR]         = {NULL,     NULL,   PREC_NONE},
	[TOKEN_EOF]           = {NULL,     NULL,   PREC_NONE},
};


static ParseRule* parse_rule_get(TokenType type) {
	return &rules[type];
}


static void parse_precedence(Precedence precedence) {
	advance();
	ParseFn prefix_rule = parse_rule_get(parser.previous.type)->prefix;
	if (prefix_rule == NULL) {
		error("Expect expression.");
		return;
	}

	bool can_assign = precedence <= PREC_ASSIGNMENT;
	prefix_rule(can_assign);

	while (precedence <= parse_rule_get(parser.current.type)->precedence) {
		advance();
		ParseFn infix_rule = parse_rule_get(parser.previous.type)->infix;
		infix_rule(can_assign);
	}

	if (can_assign && match(TOKEN_EQUAL)) {
		error("Invalid assignment target.");
	}
}


bool compile(const char *source, Chunk* chunk) {
	scanner_init(source);
	compiling_chunk = chunk;
	parser.had_error = false;
	parser.panic_mode = false;

	advance();

	while (!match(TOKEN_EOF)) {
		declaration();
	}

	compiler_end();
	return !parser.had_error;
}

