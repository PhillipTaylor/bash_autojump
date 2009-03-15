
#ifndef AUTOJUMP_H
#define AUTOJUMP_H

void autojump_init();
void autojump_exit();

//We expose this function to 'cd' so we can
//recieve notifications of when the directory
//has changed.
void autojump_directory_changed(char *new_path);

//This is the implementation for the built in 'j' command
//we take the criteria and return the best matching directory
//or NULL if there is no match.
char *autojump_jump(char *criteria);

//This is the implementation for the built in 'jumpstat' command
//It prints out our internal score table.
void autojump_jumpstat();

#endif
