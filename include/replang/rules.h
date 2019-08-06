#pragma once
#include <stringhelper/string.h>

#define RULE_INITIALIZER (rule_t){STRING_INITIALIZER, STRING_INITIALIZER}
#define RULES_INITIALIZER (rules_t){malloc(0), 0}

typedef struct{
	string_t in;
	string_t out;
} rule_t;

typedef struct{
	rule_t* _rules;
	unsigned int count;
} rules_t;

rules_t* rules_add_rule(rules_t* rules, rule_t new_rule);

void rules_apply(string_t* data, rules_t rules, int starti);

void rules_free(rules_t* rules);

void rule_free(rule_t* rule);
