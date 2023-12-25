/* C markdown parser
* Written by Oliver D'Esposiot (O-Despo)
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <err.h>

typedef enum {B_NONE, HTML, HR, HEADER, UL_LIST, OR_LIST, B_CODE, P} Block;
typedef enum {L_NONE, EMPHASIS, ODE} In_Line;
typedef enum {START, MIDDLE, END} Line_Pos;

typedef struct {
    Block block;
    In_Line in_line;
    Line_Pos line_pos;
    Line_Pos last_pos;
} State;

short format_break(State *state, FILE *in_file, char c){
    /* short format_break(State *state, FILE *in_file, char c)
     *
     * Two END charecters in a row then insert a blank line. Write to file.
     * On format reuturns 1. Dose not effect state.
     */
    short formated = 0;
    if(state->line_pos == END && state->last_pos == END){
        fputs("</br>", stdout);
        formated = 1;
    }

    return formated;
}

void set_line_pos(State *state, char c){
    /* short set_line_pos(State *state, char c)
     *
     * Sets the current line position and the last pos. Mods state.
     * Dose not write to output. Dose not return value.
     * If `\n` the set END. If END set START. If START and not `\n` set MIDDLE.
     * When POS is set save last POS.
     */
    if(c == '\n'){
        state->last_pos = state->line_pos;
        state->line_pos = END;    
    } else if (state->line_pos == END) {
        state->last_pos = state->line_pos;
        state->line_pos = START;
    } else if (state->line_pos == START) {
        state->last_pos = state->line_pos;
        state->line_pos = MIDDLE; 
    }
}

int main(int argc, char *argv[]){
    int c;
    FILE *in_file;
    State state = {B_NONE, L_NONE, END, END}; /* Set the frist line to line start */
    
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
    
    /* Start processing file */
    while(EOF != (c = fgetc(in_file))){
        set_line_pos(&state, c);
        if(!format_break(&state, in_file, c)){
        }
        /* PROCESS */
        fputc(c, stdout);

    }
    
    
    fprintf(stdout, "*****TEST Output complete*****\n");
    return 0;
}

