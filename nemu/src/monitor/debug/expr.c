#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum
{
	NOTYPE = 256,
	EQ,
	NEQ,
	ADD,
	SUB,
	MUL,
	DIV,
	MOD,
	LP,
	RP,

	NUMBER,

	UNKNOWN_TOKEN
};

static struct rule
{
	char *regex;
	int token_type;
} rules[] = {
	{" +", NOTYPE}, // spaces
	{"==", EQ},		// equal
	{"!=", EQ},		// equal

	{"\\+", ADD}, // add
	{"-", SUB},   //sub
	{"\\*", MUL}, //mul
	{"/", DIV},   //div
	{"%", MOD},   //mod
	{"\\(", LP},  //lp
	{"\\)", RP},  //rp

	{"[0-9]+", NUMBER} // number
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]))

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex()
{
	int i;
	char error_msg[128];
	int ret;

	for (i = 0; i < NR_REGEX; i++)
	{
		ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
		if (ret != 0)
		{
			regerror(ret, &re[i], error_msg, 128);
			Assert(ret == 0, "regex compilation failed: %s\n%s", error_msg, rules[i].regex);
		}
	}
}

typedef struct token
{
	int type;
	char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e)
{
	int position = 0;
	int i;
	regmatch_t pmatch;

	nr_token = 0;

	while (e[position] != '\0')
	{
		/* Try all rules one by one. */
		for (i = 0; i < NR_REGEX; i++)
		{
			if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0)
			{
				char *substr_start = e + position;
				int substr_len = pmatch.rm_eo;

				// Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position, substr_len, substr_len, substr_start);
				position += substr_len;

				/* TODO: Now a new token is recognized with rules[i]. Add codes
				 * to record the token in the array ``tokens''. For certain 
				 * types of tokens, some extra actions should be performed.
				 */
				int tt = rules[i].token_type;
				switch (tt)
				{
				case NOTYPE:
					break;

				default:
					tokens[nr_token].type = rules[i].token_type;
					strncpy(tokens[nr_token].str, substr_start, substr_len);
					tokens[nr_token].str[substr_len] = '\0';
					nr_token++;
					break;
				}
				break;
			}
		}

		if (i == NR_REGEX)
		{
			printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
			return false;
		}
	}

	return true;
}

static bool check_parentheses(int l, int r)
{
	if (tokens[l].str != '(' || tokens[r].str != ')') return false;
	int i, lc = 0, rc = 0;
	for ( i = l + 1; i < r; i++)
	{
		if (token[i].type == '(') lc++;
		if (token[i].type == ')') rc++;
		if (rc > lc) return false;
	}
	if (lc == rc)
		return true;
	return false;			
}

int dominant_operator(int l, int r) {
	int i,j;
	
}

// <expr> ::= <number>        # 一个数是表达式
//     | "(" <expr> ")"    # 在表达式两边加个括号也是表达式
//     | <expr> "+" <expr>    # 两个表达式相加也是表达式
//     | <expr> "-" <expr>    # 接下来你全懂了
//     | <expr> "*" <expr>
//     | <expr> "/" <expr>
//求值
static uint32_t eval(int l, int r)
{
	uint32_t num = 0;
	if (l > r)
	{
		Assert(true, "bad expression");
		/* bad expression */
	}
	else if (l == r)
	{
		sscanf(tokens[l].str, "%d", &num);
		return num;
	}
	else if (check_parentheses(l, r))
	{
		return eval(l + 1, r - 1);
	}
	else
	{

	}
}

uint32_t expr(char *e, bool *success)
{
	if (!make_token(e))
	{
		*success = false;
		return 0;
	}

	printf("tokens: \n");

	for (size_t i = 0; i < nr_token; i++)
	{
		Token t = tokens[i];
		printf("type : %d,\tvalue: %s\n", t.type, t.str);
	}

	return 0;
}
