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

	NUMBER,

	UNKNOWN_TOKEN
};

static struct rule
{
	char *regex;
	int token_type;
	int priority;
} rules[] = {
	{" +", NOTYPE, -1}, // spaces
	{"==", EQ, 7},		// equal
	{"!=", NEQ, 7},		// not equal

	{"\\+", '+', 4}, // add
	{"-", '-', 4},   //sub
	{"\\*", '*', 3}, //mul
	{"/", '/', 3},   //div
	{"%", '%', 3},   //mod
	{"\\(", '(', -1},  //lp
	{"\\)", ')', -1},  //rp

	{"[0-9]+", NUMBER, -1} // number
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
	int token_type;
	char str[32];
	int priority;
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
				int tt = rules[i].token_type;
				switch (tt)
					{
						case NOTYPE:
						break;

					default:
						tokens[nr_token].token_type = rules[i].token_type;
						tokens[nr_token].priority = rules[i].priority;
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
	if (tokens[l].token_type != '(' || tokens[r].token_type != ')') return false;
	int i, lc = 0, rc = 0;
	for ( i = l + 1; i < r; i++)
	{
		if (tokens[i].token_type == '(') lc++;
		if (tokens[i].token_type == ')') rc++;
		if (rc > lc) return false;
	}
	if (lc == rc)
		return true;
	return false;			
}

int dominant_operator(int l, int r) {
	if (l > r) assert(0);
	int op = l;
	int max_priority = -1;
	while (l < r)
	{
		int priority = tokens[l++].priority;
		if (max_priority <= priority)
		{
			max_priority = priority;
			op = l;
		}
	}
	return op;	
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
	if (l > r)
	{
		Assert(true, "bad expression");
		/* bad expression */
	}
	else if (l == r)
	{
		uint32_t num = 0;
		sscanf(tokens[l].str, "%d", &num);
		return num;
	}
	else if (check_parentheses(l, r))
	{
		return eval(l + 1, r - 1);
	}
	else
	{
		int op = dominant_operator(l, r);
		printf("the op is %d\t%s ", op, tokens[op].str);
	}
	return 0;
}
// make_tokenxx
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
		printf("type : %d,\tvalue: %s\n", t.token_type, t.str);
	}
	eval(0, nr_token);
	return 0;
}
