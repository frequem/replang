#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>

#include <stringhelper/string.h>
#include <replang/rules.h>

#define DEFAULT_ROUNDS 10

#define FILE_CHUNKSIZE 1024

void usage(char* pname){
	fprintf(stderr, "Usage: %s [-r rounds] [-i inputfile] [-o outputfile] [-n]\n", pname);
	fprintf(stderr, "Default: %s -r %i -i in.rl -o out.rl\n", pname, DEFAULT_ROUNDS);
}

void read_file(FILE* fin, string_t* data){
	char buf[FILE_CHUNKSIZE];
	size_t nread;
	while((nread = fread(buf, sizeof(char), FILE_CHUNKSIZE, fin)) > 0){
		string_append(data, data, string_buf(buf, nread));
	}
}

int main(int argc, char** argv){
	int rounds = DEFAULT_ROUNDS;
	int norules = 0;
	char* fn_in = NULL;
	char* fn_out = NULL;
	
	char c;
	while((c = getopt(argc, argv, "r:i:o:n")) != -1){
		switch(c){
			case 'r':
				rounds = strtol(optarg, NULL, 10);
				break;
			case 'i':
				fn_in = optarg;
				break;
			case 'o':
				fn_out = optarg;
				break;
			case 'n':
				norules = 1;
				break;
			default:
				usage(argv[0]);
				return EXIT_FAILURE;
		}
	}
	
	FILE* f_in = stdin;
	FILE* f_out = stdout;
	
	if(fn_in != NULL){
		f_in = fopen(fn_in, "r");
		if(f_in == NULL){
			fprintf(stderr, "%s: Error opening input file %s\n", argv[0], fn_in);
			return EXIT_FAILURE;
		}
	}
	
	if(fn_out != NULL){
		f_out = fopen(fn_out, "w");
		if(f_out == NULL){
			fprintf(stderr, "%s: Error opening output file %s\n", argv[0], fn_out);
			fclose(f_in);
			return EXIT_FAILURE;
		}
	}
	string_t data = STRING_INITIALIZER;
	read_file(f_in, &data);
	
	fclose(f_in);
	
	rules_t rules = RULES_INITIALIZER;
	rule_t rule;
	
	int i = string_find(&data, string_cstr("#replace "), 0);
	int j = 0;
	while(i < string_len(&data)){
		if(i==0 || string_charAt(&data, i-1) == '\n'){
			rule = RULE_INITIALIZER;
			
			j = string_find(&data, string_char(' '), i+9);
			string_substr(&rule.in, &data, i+9, j-i-9);
			
			i = string_find(&data, string_char('\n'), j+1);
			string_substr(&rule.out, &data, j+1, i-j-1);
						
			rules_add_rule(&rules, rule);
			j = i+1;
		}
		i = string_find(&data, string_cstr("#replace "), i);
	}
	
	//add newline
	rule = RULE_INITIALIZER;
	string_copy(&rule.in, string_cstr("\\n"));
	string_copy(&rule.out, string_char('\n'));
	rules_add_rule(&rules, rule);
	
	//add tab
	rule = RULE_INITIALIZER;
	string_copy(&rule.in, string_cstr("\\t"));
	string_copy(&rule.out, string_char('\t'));
	rules_add_rule(&rules, rule);
	
	if(norules){
		string_substr(&data, &data, j, -1);
		j = 0;
	}
	
	for(i = 0; i<rounds; i++){
		rules_apply(&data, rules, j);
	}
	
	rules_free(&rules);
	
	string_write(fileno(f_out), &data);
	
	string_free(&data);
	fclose(f_out);
	
	return EXIT_SUCCESS;
}
