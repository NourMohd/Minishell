
/*
 * CS354: Shell project
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>

#include "command.h"

SimpleCommand::SimpleCommand()
{
	// Creat available space for 5 arguments
	_numberOfAvailableArguments = 5;
	_numberOfArguments = 0;
	_arguments = (char **)malloc(_numberOfAvailableArguments * sizeof(char *));
}

void SimpleCommand::insertArgument(char *argument)
{
	if (_numberOfAvailableArguments == _numberOfArguments + 1)
	{
		// Double the available space
		_numberOfAvailableArguments *= 2;
		_arguments = (char **)realloc(_arguments,
									  _numberOfAvailableArguments * sizeof(char *));
	}

	_arguments[_numberOfArguments] = argument;

	// Add NULL argument at the end
	_arguments[_numberOfArguments + 1] = NULL;

	_numberOfArguments++;
}

Command::Command()
{
	// Create available space for one simple command
	_numberOfAvailableSimpleCommands = 1;
	_simpleCommands = (SimpleCommand **)
		malloc(_numberOfSimpleCommands * sizeof(SimpleCommand *));

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_background = 0;
}

void Command::insertSimpleCommand(SimpleCommand *simpleCommand)
{
	if (_numberOfAvailableSimpleCommands == _numberOfSimpleCommands)
	{
		_numberOfAvailableSimpleCommands *= 2;
		_simpleCommands = (SimpleCommand **)realloc(_simpleCommands,
													_numberOfAvailableSimpleCommands * sizeof(SimpleCommand *));
	}

	_simpleCommands[_numberOfSimpleCommands] = simpleCommand;
	_numberOfSimpleCommands++;
}

void Command::clear()
{
	for (int i = 0; i < _numberOfSimpleCommands; i++)
	{
		for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++)
		{
			free(_simpleCommands[i]->_arguments[j]);
		}

		free(_simpleCommands[i]->_arguments);
		free(_simpleCommands[i]);
	}

	if (_outFile)
	{
		free(_outFile);
	}

	if (_inputFile)
	{
		free(_inputFile);
	}

	if (_errFile)
	{
		free(_errFile);
	}

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_background = 0;
}

void Command::print()
{
	printf("\n\n");
	printf("              COMMAND TABLE                \n");
	printf("\n");
	printf("  #   Simple Commands\n");
	printf("  --- ----------------------------------------------------------\n");

	for (int i = 0; i < _numberOfSimpleCommands; i++)
	{
		printf("  %-3d ", i);
		for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++)
		{
			printf("\"%s\" \t", _simpleCommands[i]->_arguments[j]);
		}
	}

	printf("\n\n");
	printf("  Output       Input        Error        Background\n");
	printf("  ------------ ------------ ------------ ------------\n");
	printf("  %-12s %-12s %-12s %-12s\n", _outFile ? _outFile : "default",
		   _inputFile ? _inputFile : "default", _errFile ? _errFile : "default",
		   _background ? "YES" : "NO");
	printf("\n\n");
}

void Command::execute()
{
	// Don't do anything if there are no simple commands
	if (_numberOfSimpleCommands == 0)
	{
		prompt();
		return;
	}

	// Print contents of Command data structure
	print();

	// execution
	int defaultin = dup( 0 );		// Input:    defaultin
	int defaultout = dup( 1 );		// Output:   file
	int defaulterr = dup( 2 );		// Error:    defaulterr

	int outfd = -1, infd = -1, errfd = -1;
	// Create file descriptor
	if (_currentCommand._outFile)
	{
		outfd = creat(_currentCommand._outFile, 0666);
	}

	if (_currentCommand._inputFile)
	{
		infd = open(_inputFile, O_RDONLY, 0666);
	}

	if (_currentCommand._errFile)
	{
		errfd = creat(_currentCommand._errFile, 0666);
	}

	// Redirect output to the created utfile instead off printing to stdout
	if (outfd > 0)
	{
		dup2(outfd, 1);
		close(outfd);
	}

	// Redirect input

	if (infd > 0)
	{
		dup2(infd, 0);
		close(infd);
	}

	// Redirect err
	if (errfd > 0)
	{
		dup2(errfd, 2);
		close(errfd);
	}

	// Create new process for command
	const char *command_name = _currentSimpleCommand->_arguments[0];
	int pid = fork();
	if (pid == -1)
	{
		perror("error\n");
		exit(2);
	}

	if (pid == 0)
	{
		// Child

		// close file descriptors that are not needed
		close(outfd);
		close(infd);
		close(errfd);
		close(defaultin);
		close(defaultout);
		close(defaulterr);
		execvp(command_name, _currentSimpleCommand->_arguments);

		// exec() is not suppose to return, something went wrong
		perror("error");
		exit(2);
	}
	// if process is not a background process
	if (_currentCommand._background == 0)
	{
		waitpid(pid, 0, 0);
	}

	dup2(defaultin, 0);
	dup2(defaultout, 1);
	dup2(defaulterr, 2);

	// // Close file descriptors that are not needed
	close(outfd);
	close(infd);
	close(errfd);
	close(defaultin);
	close(defaultout);
	close(defaulterr);

	// Clear to prepare for next command
	clear();

	// Print new prompt
	prompt();
}

// Shell implementation

void Command::prompt()
{
	printf("myshell>");
	fflush(stdout);
}

Command Command::_currentCommand;
SimpleCommand *Command::_currentSimpleCommand;

int yyparse(void);

int main()
{
	Command::_currentCommand.prompt();
	yyparse();
	return 0;
}
