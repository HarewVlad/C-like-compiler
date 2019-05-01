const char *int_keyword;
const char *double_keyword;
const char *char_keyword;
const char *bool_keyword;
const char *if_keyword;
const char *else_keyword;
const char *while_keyword;

const char **keywords;
const char *first_keyword;
const char *last_keyword;

#define KEYWORD(x) x##_keyword = str_intern(#x, strlen(#x)); buf_push(keywords, x##_keyword);

void init_keywords()
{
	KEYWORD(int);
	KEYWORD(double);
	KEYWORD(char);
	KEYWORD(bool);
	KEYWORD(if);
	KEYWORD(else);
	KEYWORD(while);

	first_keyword = int_keyword;
	last_keyword = while_keyword;
}

#undef KEYWORD

bool is_keyword_name(const char *name) // TODO: int -> bool
{
	return name >= first_keyword && name <= last_keyword;
}

char *stream;

typedef enum TokenKind
{
	// Types
	TOKEN_NONE = 0,
	TOKEN_INT,
	TOKEN_DOUBLE,
	TOKEN_CHAR,
	TOKEN_BOOL,
	TOKEN_NAME,
	TOKEN_KEYWORD,
	// CMP
	TOKEN_EQ,
	TOKEN_EQ_EQ,
	// Add, mul ...
	TOKEN_ADD,
	TOKEN_ADD_EQ,
	TOKEN_SUB,
	TOKEN_SUB_EQ,
	TOKEN_MUL,
	TOKEN_MUL_EQ,
	TOKEN_DIV,
	TOKEN_DIV_EQ,
	TOKEN_INC,
	TOKEN_DEC,
} TokenKind;

const char *token_to_char[] =
{
	[TOKEN_INT] = "int",
	[TOKEN_CHAR] = "char",
	[TOKEN_DOUBLE] = "double",
	[TOKEN_EQ] = "=",
	[TOKEN_EQ_EQ] = "==",
	[TOKEN_ADD] = "+",
	[TOKEN_ADD_EQ] = "+=",
	[TOKEN_SUB] = "-",
	[TOKEN_SUB_EQ] = "-=",
	[TOKEN_MUL] = "*",
	[TOKEN_MUL_EQ] = "*=",
	[TOKEN_DIV] = "/",
	[TOKEN_DIV_EQ] = "/=",
	[TOKEN_INC] = "++",
	[TOKEN_DEC] = "--",
};

typedef struct Token
{
	TokenKind kind;
	union
	{
		int64_t int_val;
		double double_val;
		const char *name;
		char char_val;
	};
} Token;

Token token;

#define case1(c, k) \
	case c: \
		token.kind = k; \
		stream++; \
		break; \

#define case2(c1, k1, c2, k2) \
	case c1: \
		token.kind = k1; \
		stream++; \
		if (*stream == c2) \
		{ \
			token.kind = k2; \
		} \
		stream++; \
		break; \

#define case3(c1, k1, c2, k2, c3, k3) \
	case c1: \
		token.kind = k1; \
		stream++; \
		if (*stream == c2) \
		{ \
			token.kind = k2; \
		} \
		else if (*stream == c3) \
		{ \
			token.kind = k3; \
		} \
		stream++; \
		break; \

void next()
{
repeat:
	token.kind = TOKEN_NONE;
	const char *begin = stream;
	const char *end = NULL;
	switch (*stream)
	{
	case ' ': case '\n': case '\r': case '\t': case '\v':
		stream++;
		goto repeat;
	case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': case '0':
		begin = stream;
		int64_t int_val = 0;
		while (isdigit(*stream))
		{
			int_val *= 10;
			int_val += *stream++ - '0';
		}
		if (*stream == '.')
		{
			stream++;
			while (isdigit(*stream))
			{
				stream++;
			}
			end = stream;

			stream = begin;
			double double_val = strtod(stream, NULL);
			token.kind = TOKEN_DOUBLE;
			token.double_val = double_val;

			stream = end;
		}
		else
		{
			token.kind = TOKEN_INT;
			token.int_val = int_val;
		}
		break;
	case '.':
		stream++;
		begin = stream;
		while (isdigit(*stream))
		{
			stream++;
		}
		end = stream;

		stream = begin;

		double double_val = strtod(stream, NULL);
		token.kind = TOKEN_DOUBLE;
		token.double_val = double_val;

		stream = end;
		break;
	case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i': case 'j':
	case 'k': case 'l': case 'm': case 'n': case 'o': case 'p': case 'q': case 'r': case 's': case 't':
	case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
	case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H': case 'I': case 'J':
	case 'K': case 'L': case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T':
	case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
	case '_':
		char *start = stream;
		while (isalnum(*stream) || *stream == '_')
		{
			stream++;
		}
		token.name = str_intern(start, stream - start);
		token.kind = is_keyword_name(token.name) ? TOKEN_KEYWORD : TOKEN_NAME;
		break;
	case '\'':
		assert(*stream++ == '\'');
		token.kind = TOKEN_CHAR;
		token.char_val = *stream++;
		assert(*stream++ == '\'');
		break;
	case2('=', TOKEN_EQ, '=', TOKEN_EQ_EQ);
	case3('+', TOKEN_ADD, '+', TOKEN_INC, '=', TOKEN_ADD_EQ);
	case3('-', TOKEN_SUB, '-', TOKEN_DEC, '=', TOKEN_SUB_EQ);
	case2('*', TOKEN_MUL, '=', TOKEN_MUL_EQ);
	case2('/', TOKEN_DIV, '=', TOKEN_DIV_EQ);
	default:
		token.kind = *stream;
		stream++;
		break;
	}
}

#undef case1
#undef case2
#undef case3
