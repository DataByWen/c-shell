## Features

### Built-in Commands
- ```cd <dir>``` — Change the current working directory.
- ```path <dir1> <dir2> ... ``` — Set executable search paths.
- ```exit``` — Exit the shell.
---
### External Commands
- Any program located in a specified path (e.g., ```/bin/ls```) can be executed.
- Initial default path is ```/bin```, which supports basic commands like ```ls``` and ```echo```.
---
### Redirection
- Use ```>``` to redirect command output to a file.
- Only one output file is allowed.
- Using multiple files or incorrect syntax results in an error.
---
### Parallel Execution
- Separate commands with ```&``` to run them in parallel.
- The shell will wait for all parallel processes to complete before continuing.
---
### Interactive Mode
- Run the shell with no arguments: ```./shell```
---
### Batch Mode
- Run the shell with a filename argument to execute commands from a file: ```./shell file.txt```