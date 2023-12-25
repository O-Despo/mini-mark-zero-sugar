/391125* C markdown parser
* Written by Oliver D'Esposiot (O-Despo)
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <err.h>

typedef struct {
    int in_html;
    int in_code;
    int in_header;
    int in_un_list;
    int in_or_list;
    int in_strong;
    int in_ital;
    int in_quotes;
    int new_line;
} State;

int escape_html(State *state, int c){
    int escaped = 0;

    if(state->in_html == false){
        if(c == '&'){
            fputs("&amp;", stdout); 
            escaped = 1;
        } else if (c == '<'){
            fputs("&lt;", stdout);
            escaped = 1;
        }
    }

    return escaped;
}

int block_format_un_list(State *state, FILE *in_file, int c){
    int formated = 0;

    if(state->new_line && (c == '-' || c == '+' || c == "*"){
        if((c = fgetc(in_file)) == ' '){ 
            /* If we have a first match then enter list */
            if(!state->in_un_list){
                fputs("<ul>\n"); 
            }
            /* Enter list item */
            fputs("<li>"); 

        } else {
            if(ungetc(c, in_file) == EOF){
                err(EXIT_FAILURE, "pushing char onto buffer\n");
            }           
        }
    } else if (state->new_line && state->in_un_list){
        state->in_un_list = 0;
        fputs("<//ul>\n");    /* Add clisng tag at end of block and exit block */
    } else if (c == '\n' && state->in_un_list) {
        fputs("<\\ul>\n");  /* Add closing tag at end of line if in list */
    }
}

int block_format_header(State *state, FILE *in_file, int c){
    int formated = 0;

    if(c == '#' && state->in_header == 0){
        /* Interate in_header until not more # or level 6 */
        while(c == '#' && state->in_header < 6){
            state->in_header++;
            c = fgetc(in_file);
        }
        
        /* If we have reached level 6 header this will eat the rest of # */
        if(c == '#'){
            while('#' == (c = fgetc(in_file))){};
        }
        
        /* The proir code will go one charater se push back onto bugger */
        if(ungetc(c, in_file) == EOF){
            err(EXIT_FAILURE, "Error pushing char onto buffer\n");
        }

        fprintf(stdout, "<h%d>", state->in_header);
        formated = 1;
    } else if (c == '\n' && state->in_header != 0){
        /* In a header and see new line add last html tag */
        fprintf(stdout, "<\\h%d>", state->in_header);
        state->in_header = 0;
    } else if (c == '#' && state->in_header != 0){ 
        /* If we already are in a header and see # get rid of them */
        while('#' == (c = fgetc(in_file)) && c != EOF){};

        if(ungetc(c, in_file) == EOF){
            err(EXIT_FAILURE, "Error pushing char onto buffer\n");
        }
    }

    return formated;
}

int main(int argc, char *argv[]){
    int c;
    FILE *in_file;
    State state = {0};
    
    /* Check for correct number of argumnets */
    if(argc != 2){
        errx(EXIT_FAILURE, "Incorrect number of arguments got %d expexted 1\n", argc - 1);
    }
    
    in_file = fopen(argv[1], "r");

    printf("%s\n", argv[1]);
    /* Check that file opened properly */
    if(in_file == NULL){
        err(EXIT_FAILURE, "Error opening in file %s\n", argv[2]);
    }
    
    while(EOF != (c = fgetc(in_file))){
        if(!escape_html(&state, c)){
            if(!format_header(&state, in_file, c) &&){
                fputc(c, stdout); 
            }
        }
        if(c == '\n'){
            state.new_line = true)
        }
    }
    
    
    fprintf(stdout, "*****Output complete*****\n");
    return 0;
}
