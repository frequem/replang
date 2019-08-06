#include <replang/rules.h>

rules_t* rules_add_rule(rules_t* rules, rule_t new_rule){
	rules->_rules = realloc(rules->_rules, sizeof(rule_t)*(rules->count+1));
	rules->_rules[rules->count] = new_rule;
	rules->count++;
	return rules;
}

int rule_findnext(string_t* data, rules_t rules, int starti, rule_t** rule_out){
	rule_t* rule = &rules._rules[0];
	int i, min_i = string_len(*data);
	for(int k=0; k<rules.count; k++){
		i = string_find(*data, rules._rules[k].in, starti);
		if(i < min_i){
			min_i = i;
			rule = &rules._rules[k];
		}
	}
	*rule_out = rule;
	return min_i;
}

void remove_braces(string_t* data, int starti){
	int i = starti;
	int bs, be;
	while(i < string_len(*data)){
		bs = string_replace(data, string_char('{'), string_cstr(""), i);
		bs--;
		be = string_find(*data, string_char('}'), bs+1);
		while((bs = string_find(*data, string_char('{'), bs+1)) < be){
			be = string_find(*data, string_char('}'), be+1);
		}
		i = string_replace(data, string_char('}'), string_cstr(""), be);
	}
}

int inside_braces(string_t* data, int starti, int rulei){
	int i = starti;
	int bc = 0;
	while(i < rulei){
		i = string_find(*data, string_char('{'), i+1);
		if(i < rulei)
			bc++;
	}
	i = starti;
	while(i < rulei){
		i = string_find(*data, string_char('}'), i+1);
		if(i < rulei)
			bc--;
	}
	return bc != 0;
}

void rules_apply(string_t* data, rules_t rules, int starti){
	rule_t* rule;
	int i = starti;
	int j;
	while(i < string_len(*data)){
		j = rule_findnext(data, rules, i, &rule);
		if(inside_braces(data, starti, j)){
			i = j+1;
			continue;
		}
		i = string_replace(data, rule->in, rule->out, i);
		i += string_len(rule->out);
	}
	remove_braces(data, starti);
}

void rules_free(rules_t* rules){
	for(int i=0; i<rules->count; i++)
		rule_free(&rules->_rules[i]);
	free(rules->_rules);
}

void rule_free(rule_t* rule){
	string_free(&rule->in);
	string_free(&rule->out);
}
