#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>

void	ft_putstr_fd2(char *str, char *arg)
{
	// write str stderr
	while (*str)
		write(2, str++, 1);
	// if arg isn't null, write arg to stderr
	if (arg)
		while (*arg)
			write(2, arg++, 1);
	write(2, "\n", 1);
}

void	ft_execute(char **argv, int i, int tmp_fd, char **env)
{
	// overwrite ; or | or NULL with NULL to use the array as input for execve
	// we are here in the child so it has no impact in the parent process.
	argv[i] = NULL;
	// We redirect stdin using dup2(). 
	// The child process needs its stdin to come from the previous command's output (via tmp_fd).
	dup2(tmp_fd, STDIN_FILENO);
	// After dup2(tmp_fd, STDIN_FILENO), we have two file descriptors pointing to the same
  	// resource. We close tmp_fd to avoid file descriptor leaks - crucial for handling hundreds of
  	// pipes
	close(tmp_fd);
	// we execute the command
	execve(argv[0], argv, env);
	// as execve replace the current process image, we are noit supposed to come back here
	// except if an error occured, in that case, we print the error
	ft_putstr_fd2("error: cannot execute ", argv[0]);
	// and we exit the program with 1
	exit(1);
}

int	main(int argc, char **argv, char **env)
{
	int	i;
	int fd[2];
	int	tmp_fd; // to store 
	(void)argc; // avoid compilation error

	tmp_fd = dup(STDIN_FILENO); // we are saving STDIN // ? what for ?
	i = 0;
	while (argv[i] && argv[i + 1]) // iterate til the 2nd to last
	{
		// `argv = &argv[i + 1]` moves the argv pointer.
		// If i points to "|", then argv[0] now points to the command after the pipe.
		// ! The first iteration skip the program name
		argv = &argv[i + 1];
		// We reset i to start parsing the new command segment from index 0
		i = 0;
		// incremente i as long as argv is not ';' nor '|'
		while (argv[i] && strcmp(argv[i], ";") && strcmp(argv[i], "|"))
			i++;
		// cd
		if (strcmp(argv[0], "cd") == 0)
		{
			if (i != 2) // if there is not 2 tokens (cd + arg)
				ft_putstr_fd2("error: cd: bad arguments", NULL);
			else if (chdir(argv[1]) != 0) // if there is an error changing directory
				ft_putstr_fd2("error: cd: cannot change directory to ", argv[1]);
		}
		// if there is at least 1 token, exec in stdout
		else if (i != 0 && (argv[i] == NULL || strcmp(argv[i], ";") == 0))
		{
			if (fork() == 0) // In the child process, execute the command
				ft_execute(argv, i, tmp_fd, env);
			else // if the fork() fail
			{
				close(tmp_fd); // close the duplicated stdin
				// This waits for all child processes to finish. waitpid(-1, NULL, WUNTRACED) waits for any
				// child.The loop continues until no more children exist(returns - 1).
				while (waitpid(-1, NULL, WUNTRACED) != -1);
				// After a sequential command (;), we need fresh stdin for the next command. 
				// We closed the old tmp_fd and create a new one pointing to original stdin.
				tmp_fd = dup(STDIN_FILENO);
			}
		}
		// pipe
		else if (i != 0 && strcmp(argv[i], "|") == 0)
		{
			// create the pipe
			pipe(fd);
			if (fork() == 0) // In the child process
			{
				dup2(fd[1], STDOUT_FILENO);
				// After dup2(fd[1], STDOUT_FILENO), stdout is redirected to the pipe's write end. 
				// We close both fd[0] and fd[1] because : 
				// 		- fd[1] is duplicated to stdout, so original is unneeded
				// 		- fd[0](read end) isn't used by this child
				close(fd[0]);
				close(fd[1]);
				ft_execute(argv, i, tmp_fd, env);
			}
			else
			{
				close(fd[1]);
				close(tmp_fd);
				tmp_fd = fd[0];
			}
		}
	}
	close(tmp_fd);
	return (0);
}