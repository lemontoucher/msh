#include "include/shellFunctions.h"

// TODO: Make this non-global
int background = 0;

/**
 * Displays information about the shell
 *
 * @returns nothing
 */
void showShellMessage()
{
    puts("\t   __  ___ __  __ ____ __ __");
    puts("\t  /  |/  // / / // __// // /");
    puts("\t / /|_/ // /_/ /_\\ \\ / _  /");
    puts("\t/_/  /_/ \\____//___//_//_/\n");
    puts("\tTHE MODULAR UNIVERSAL SHELL\n");
    puts("Welcome to the MUSH shell!");
    puts("Created by Brandon Fairchild\n");
    puts("Enter `help` at any time to get extra information.");
    puts("Type `exit` to close the shell.\n");
}

/**
 * Displays the prompt symbol to let the user know they can enter
 * a new command.
 *
 * By default, the prompt symbol is provided by PROMPT_SYM.
 *
 * @returns nothing
 */
void showPrompt()
{
    printf("%c ", PROMPT_SYM);
}

/**
 * Starts the execution of the shell.
 *
 * @returns nothing
 */
void run()
{
    showShellMessage();
    getInput();
}

/**
 * Gets input from the user until the command `exit` is entered.
 *
 * @returns nothing
 */
void getInput()
{
    char command[MAX_ARGUMENTS];

    do
    {
        showPrompt();
        fgets(command, MAX_ARGUMENTS-1, stdin);
        runCommand(command);

    } while(strcmp(command, "exit"));
}

/**
 * Parses a given command into an array of tokens using strtok()
 *
 * @returns A pointer to an array of tokens on the heap from the
 *          given command.
 *
 *          i.e. `ls -la` will be stored as { "ls", "-la", NULL }
 */
void parseCommand(char* command, char* arguments[])
{
    char* tokenArray[strlen(command)];
    char* token = strtok(command, " ");

    int i = 0;

    while(token != NULL)
    {
        char* start = token;

        for(; *token != '\0'; token++)
        {
  	    if(*token == '\n')
	    {
		*token = '\0';
	    }
	}

	// Reset to the start of the string
	token = start;

	tokenArray[i] = token;

	token = strtok(NULL, " ");
	i++;
    }

    tokenArray[i] = NULL;

    for(i = 0; tokenArray[i] != NULL; i++)
    {
	// Exclude empty strings from the returned array
	// This could cause commands such as `ls` to be passed nothing if there
	// are spaces after it
	if(strcmp(tokenArray[i], "") != 0)
	{
	    if(strcmp(tokenArray[i],"&") != 0)
	    {
	        arguments[i] = tokenArray[i];
	    }
	    else
	    {
	        background = 1;
	    }
        }
    }

    arguments[i] = NULL;
}

/**
 * Handles redirection setup for the shell.
 *
 * @param tokens     A character string array containing the tokens from parsing
 * @param index      The current index into the tokens array
 * @param fid        The file descriptor to modify
 * @param attributes Flags to open the file with (i.e O_CREAT | O_TRUNC, etc.)
 *
 * @returns nothing
 */
void handleRedirection(char** tokens, int* index, int fid, int attributes)
{
    // Check that this isn't the first or last token in the command
    if(*index == 0 || tokens[(*index)+1] == NULL)
    {
        return;
    }

    tokens[*index] = NULL;
    int fileId = open(tokens[*(index)+1], attributes, 0666);
    close(fid);
    dup(fileId);
    close(fileId);
    (*index)++;
}

void redirect(char** tokens)
{
    for(int i = 0; tokens[i] != NULL; i++)
    {
        if(strcmp(tokens[i], ">") == 0)
        {
            handleRedirection(tokens, &i, 1, O_CREAT | O_WRONLY | O_TRUNC);
        }

        if(strcmp(tokens[i], ">>") == 0)
        {
            handleRedirection(tokens, &i, 1, O_WRONLY | O_APPEND);
        }

        if(strcmp(tokens[i], "<") == 0)
        {
            handleRedirection(tokens, &i, 0, O_RDONLY);
        }
    }
}

void runCommand(char* commandString)
{
    // Parse the given command before running it
    char* arguments[2000];
    parseCommand(commandString, arguments);

    if(strcmp(arguments[0], "") == 0 || arguments[0] == NULL)
    {
        return;
    }
    else if(strcmp(arguments[0], "cd") == 0)
    {
        chdir(arguments[1]);
    }
    else if(strcmp(arguments[0], "help") == 0)
    {
        puts("usage:");
        puts("exit: exit the shell");
    }
    else if(strcmp(arguments[0], "exit") != 0)
    {
        pid_t pid = fork();

        if(pid == 0)
        {
            char** args = arguments;
            char commandLocation[100] = "/bin/";

            redirect(arguments);

            strncat(commandLocation, arguments[0], 92);

            if(execvp(commandLocation, args) < 0)
            {
                strcpy(commandLocation, "/usr/bin/");
                strncat(commandLocation, arguments[0], 90);

                // The command was not found in /bin, so maybe the command will
                // be found in the other location that stores programs, /usr/bin/.
                if(execvp(commandLocation, args) < 0)
                {
                    printf("\x1b[31;1m mush: `%s` - command not found. \x1b[0m\n", arguments[0]);
                }
            }

            exit(0);
        }
        else
        {
	    // Check for running a background process
	    if(!background)
	    {
                waitpid(pid, NULL, 0);
	    }
	    else
	    {
                background = 0;
                sleep(1);
                printf("%d\n", pid);
	    }
        }
    }
}