#include <replang/rules.h>
#include <unistd.h>

rules_t* rules_add_rule(rules_t* rules, rule_t new_rule){
	rules->_rules = realloc(rules->_rules, sizeof(rule_t)*(rules->count+1));
	rules->_rules[rules->count] = new_rule;
	rules->count++;
	return rules;
}

int rule_findnext(string_t* data, rules_t rules, int starti, rule_t** rule_out){
	rule_t* rule = &rules._rules[0];
	int i, min_i = string_len(data);
	for(int k=0; k<rules.count; k++){
		i = string_find(data, &rules._rules[k].in, starti);
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
	
	while(i < string_len(data)){
		bs = string_replace(data, data, string_char('{'), string_cstr(""), i);
		bs--;
		be = string_find(data, string_char('}'), bs+1);
		while((bs = string_find(data, string_char('{'), bs+1)) < be){
			be = string_find(data, string_char('}'), be+1);
		}
		i = string_replace(data, data, string_char('}'), string_cstr(""), be);
	}
}

int inside_braces(string_t* data, int starti, int rulei){
	int i = starti;
	int bc = 0;
	while(i < rulei){
		i = string_find(data, string_char('{'), i+1);
		if(i < rulei)
			bc++;
	}
	i = starti;
	while(i < rulei){
		i = string_find(data, string_char('}'), i+1);
		if(i < rulei)
			bc--;
	}
	return bc != 0;
}

void replace_math(string_t* data, int starti){
	int i = starti, j;
	int bs, be;
	int sign; //3: *, 2: /, 1: +, 0: -
	string_t left = STRING_INITIALIZER;
	string_t right = STRING_INITIALIZER;
	int is_float;
	float a, b;
	char *endptr_f, *endptr_i;
	while(i < string_len(data)){
		sign = 3;
		is_float = 0;
		bs = string_find(data, string_char('('), i);
		be = string_find(data, string_char(')'), bs+1);
		
		//* & / first to deal with scientific notation
		j = string_find(data, string_char('*'), bs+2);
		if(j >= be){
			j = string_find(data, string_char('/'), bs+2);
			sign--;
		}
		if(j >= be){
			j = string_find(data, string_char('+'), bs+2);
			sign--;
		}
		if(j >= be){
			j = string_find(data, string_char('-'), bs+2);
			sign--;
		}
		if(j >= be || inside_braces(data, starti, j)){
			i = bs+1;
			continue;
		}
		
		string_substr(&left, data, bs+1, j-bs-1);
		string_append(&left, &left, string_char('\0'));
		
		string_substr(&right, data, j+1, be-j-1);
		string_append(&right, &right, string_char('\0'));
		
		//left
		a = strtof(left.chars, &endptr_f);
		if(endptr_f[0] != '\0' && endptr_f[0] != ' '){
			i = bs+1;
			continue;
		}
		strtol(left.chars, &endptr_i, 10);
		if(endptr_f[0] != endptr_i[0]){
			is_float = 1;
		}
		
		//right
		b = strtof(right.chars, &endptr_f);
		if(endptr_f[0] != '\0' && endptr_f[0] != ' '){
			i = bs+1;
			continue;
		}
		strtol(right.chars, &endptr_i, 10);
		if(endptr_f[0] != endptr_i[0]){
			is_float = 1;
		}
		
		switch(sign){
			case 0: a = a-b; break; // -
			case 1: a = a+b; break; // +
			case 2: a = a/b; break; // /
			case 3: a = a*b; break; // *
			default: i=bs+1; continue;
		}
		
		string_erase(data, data, bs, be-bs+1);
		string_t* in = is_float?string_float(a):string_int((long long int) a);
		string_insert(data, data, in, bs);
		
		i = bs+string_len(in);
	}
	string_free(&left);
	string_free(&right);
}

void rules_apply(string_t* data, rules_t rules, int starti){
	rule_t* rule;
	int i = starti;
	int j;
	replace_math(data, starti);
	while(i < string_len(data)){
		j = rule_findnext(data, rules, i, &rule);
		if(inside_braces(data, starti, j)){
			i = j+1;
			continue;
		}
		i = string_replace(data, data, &rule->in, &rule->out, i);
		i += string_len(&rule->out);
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
