//
// Created by ninetail on 12/2/20.
//

#ifndef LAB26_28_CONSOLE_COLORS_H
#define LAB26_28_CONSOLE_COLORS_H

char *CHILD_COLOR;
char *PARENT_COLOR;
char *ERROR_COLOR;
char *DEFAULT_COLOR;

char *BLACK_COLOR;
char *RED_COLOR;
char *GREEN_COLOR;
char *YELLOW_COLOR;
char *BLUE_COLOR;
char *MAGENTA_COLOR;
char *CYAN_COLOR;
char *WHITE_COLOR;

void throw_and_exit(char *call_name);
void throw_with_code_and_exit(char *call_name, int error_code);


#endif //LAB26_28_CONSOLE_COLORS_H
